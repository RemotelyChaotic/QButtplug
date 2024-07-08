#ifndef QBUTTPLUGCLIENT_P_H
#define QBUTTPLUGCLIENT_P_H

#include "qbuttplugclient.h"
#include "qbuttplugenums.h"

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QWebSocket>

#include <atomic>
#include <vector>

inline QString qt_toString(QtButtplug::ConnectionState state)
{
  switch (state)
  {
    case QtButtplug::Disconnected:
      return "Disconnected";
    case QtButtplug::Connecting:
      return "Connecting";
    case QtButtplug::Handshake:
      return "Handshake";
    case QtButtplug::Connected:
      return "Connected";
  }
  return QString();
}

inline QString qt_toString(QtButtplug::Error err)
{
  switch (err)
  {
    case QtButtplug::ERROR_OK:
      return "ERROR_OK";
    case QtButtplug::ERROR_UNKNOWN:
      return "ERROR_UNKNOWN";
    case QtButtplug::ERROR_INIT:
      return "ERROR_INIT";
    case QtButtplug::ERROR_PING:
      return "ERROR_PING";
    case QtButtplug::ERROR_MSG:
      return "ERROR_MSG";
    case QtButtplug::ERROR_DEVICE:
      return "ERROR_DEVICE";
    case QtButtplug::ERROR_PING_TIMEOUT:
      return "ERROR_PING_TIMEOUT";
    case QtButtplug::ERROR_SOCKET_ERR:
      return "ERROR_SOCKET_ERR";
    case QtButtplug::ERROR_TIMEOUT:
      return "ERROR_SOCKET_ERR";
  }
  return "";
}

inline QDebug operator<<(QDebug dbg, const QtButtplug::ConnectionState& message)
{
  dbg.nospace() << "ConnectionState(" + qt_toString(message) + ")";
  return dbg.maybeSpace();
}

inline QDebug operator<<(QDebug dbg, const QtButtplug::Error& err)
{
  dbg.nospace() << "Error(" + qt_toString(err) + ")";
  return dbg.maybeSpace();
}

//----------------------------------------------------------------------------------------
//
class ThreadFunctionCaller : public QObject
{
  Q_OBJECT
public:
  ThreadFunctionCaller(QThread* pThread, std::function<void(void)> fn, bool bBlocking, bool& bOk) :
    QObject(nullptr),
    m_fn(fn)
  {
    bOk = true;
    moveToThread(pThread);
    auto ok = QMetaObject::invokeMethod(this, "call",
                                        bBlocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
    if (Q_UNLIKELY(!ok)) {
        qWarning() << "ThreadFunctionCaller could not call lambda.";
        bOk = false;
    }
  }

private slots:
  void call()
  {
    m_fn();
    deleteLater();
  }

private:
  std::function<void(void)> m_fn;
};

inline bool qt_callInThread(QThread* pThread, std::function<void(void)> fn, bool bBlocking = false)
{
  bool bOk = true;
  new ThreadFunctionCaller(pThread, fn, bBlocking, bOk);
  return bOk;
}

//----------------------------------------------------------------------------------------
//
QT_BEGIN_NAMESPACE

class QButtplugMessageSerializer;

class QButtplugClientPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QButtplugClient)
  QButtplugClient* const q_ptr;
  friend class QButtplugClientDevicePrivate;

public:
  QButtplugClientPrivate(QButtplugClient* const q);
  ~QButtplugClientPrivate() override;

  bool waitConnected(qint64 iTimeoutMs);
  bool waitDisconnected(qint64 iTimeoutMs);

  void connectToHost(const QHostAddress& sAddr, qint16 iPort);
  void disconnectFromHost();
  void startScan();
  void stopScan();

  QtButtplug::Error stopAllDevices();

signals:
  void connected();
  void disconnected();
  void deviceAdded(quint32 iId, QButtplugClientDevice device);
  void deviceRemoved(quint32 iId);
  void scanningFinished();
  void errorRecieved(QtButtplug::Error error);

  void q_await_response();

protected slots:
  void onDisconnect();
  void pingTimerTimeout();
  // connected with Qt::DirectConnection to socket in socket thread
  void socketError(QAbstractSocket::SocketError error);
  // connected with Qt::DirectConnection to socket in socket thread
  void startHandshake();
  // connected with Qt::DirectConnection to socket in socket thread
  void textMessageRecieved(const QString& sMessage);

protected:
  bool send(ButtplugMessageBase* pMsg, QStringList vsExpectedResponses,
            std::function<void(ButtplugMessageBase*)> fnRespHandler = nullptr);
  qint64 getNextId();

  QPointer<QWebSocket> m_pWs;
  QThread              m_wsThread;
  QTimer               m_pingTimer;
  QButtplugMessageSerializer* m_pMsgSerializer;

  QString m_sClientName = "ButtplugClient";
  QHostAddress m_hostAddr = QHostAddress(QHostAddress::LocalHost);
  qint16 m_iPort = 12345;
  QtButtplug::ButtplugProtocolVersion m_iMsgVersionUsed = QtButtplug::AnyProtocolVersion; // no version used
  QtButtplug::ButtplugProtocolVersion m_iMsgVersionSupported = QtButtplug::AnyProtocolVersion; // all versions supported
  std::atomic<QtButtplug::ConnectionState> m_connState = QtButtplug::Disconnected;
  bool m_bRunningMsgDiscovery = false;
  bool m_bIsScanning = false;

  QString m_sServerName;
  qint64 m_iMaxPingTime = 10;

  QMap<quint32, QButtplugClientDevice> m_devices;

  mutable QMutex    m_errMut;
  QtButtplug::Error m_error;
  QString           m_errorDetailString;

private:
  struct ClientPackage
  {
    qint64 m_iId;
    ButtplugMessageBase* m_pOutMsg;
    QStringList m_vsExpectedResponses;
    std::function<void(ButtplugMessageBase*)> m_fnResponseHandler;
  };

  std::map<QString, std::function<void(ButtplugMessageBase*)>> m_spontaniousMsgHandlers;
  std::vector<ClientPackage> m_clientPackageQueue;
  qint64 m_iLastId = 1;

  void q_handle_scanResponse(ButtplugMessageBase* pMsg, bool bStart);
  void q_handle_handshake_response(ButtplugMessageBase* pMsg);
  void q_handle_scanning_finished(ButtplugMessageBase* pMsg);
  void q_handle_device_list(ButtplugMessageBase* pMsg);
  void q_handle_device_added(ButtplugMessageBase* pMsg);
  void q_handle_device_removed(ButtplugMessageBase* pMsg);
  void q_handle_sensor_reading(ButtplugMessageBase* pMsg);
  void q_handle_raw_reading(ButtplugMessageBase* pMsg);
  void q_clearQueue();
  void q_setErr(QtButtplug::Error err, const QString& sErr);
  void q_resetErr();
};

QT_END_NAMESPACE

#endif // QBUTTPLUGCLIENT_P_H
