#ifndef QBUTTPLUGCLIENT_P_H
#define QBUTTPLUGCLIENT_P_H

#include "qbuttplugclient.h"
#include "qbuttplugenums.h"

#include <QObject>
#include <QMap>
#include <QString>

inline QString toString(QtButtplug::ConnectionState state)
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

inline QString toString(QtButtplug::Error err)
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
  }
  return "";
}

inline QDebug operator<<(QDebug dbg, const QtButtplug::ConnectionState& message)
{
  dbg.nospace() << "ConnectionState(" + toString(message) + ")";
  return dbg.maybeSpace();
}

inline QDebug operator<<(QDebug dbg, const QtButtplug::Error& err)
{
  dbg.nospace() << "Error(" + toString(err) + ")";
  return dbg.maybeSpace();
}

//----------------------------------------------------------------------------------------
//
QT_BEGIN_NAMESPACE

class QButtplugClientPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QButtplugClient)
  QButtplugClient* const q_ptr;
  friend class QButtplugClientDevicePrivate;

public:
  QButtplugClientPrivate(QButtplugClient* const q) :
    QObject(q),
    q_ptr(q)
  {
  }
  ~QButtplugClientPrivate() override
  {
  }

  bool waitConnected(qint64 iTimeoutMs);
  bool waitDisconnected(qint64 iTimeoutMs);

  void connect(const QHostAddress& sAddr, qint16 iPort);
  void disconnect();
  void startScan();
  void stopScan();

  QtButtplug::Error stopAllDevices();

protected:
  QString m_sClientName = "ButtplugClient";
  QHostAddress m_hostAddr = QHostAddress(QHostAddress::LocalHost);
  qint16 m_iPort = 12345;
  QtButtplug::ButtplugProtocolVersion m_iMsgVersionUsed = QtButtplug::AnyProtocolVersion; // no version used
  QtButtplug::ButtplugProtocolVersion m_iMsgVersionSupported = QtButtplug::AnyProtocolVersion; // all versions supported
  QtButtplug::ConnectionState m_connState = QtButtplug::Disconnected;
  bool m_bIsScanning = false;

  QMap<quint32, QButtplugClientDevice> m_devices;

  QtButtplug::Error m_error;
};

QT_END_NAMESPACE

#endif // QBUTTPLUGCLIENT_P_H
