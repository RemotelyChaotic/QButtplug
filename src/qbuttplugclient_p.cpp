#include "qbuttplugclient_p.h"
#include "qbuttplugmessageparsing.h"

#include <QEventLoop>
#include <QObject>
#include <QMap>
#include <QString>

#include <limits>

namespace
{
  constexpr qint32 c_iSpontaniousMsgId = 0;
  constexpr qint32 c_iGlobalTimeout = 15000;
  constexpr qint64 c_iMinPingTimeMs = 10;

  template <typename Func1>
  bool q_busy_wait(const typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                   qint64 iTimeout)
  {
    bool bTimeout = false;
    QTimer t;
    t.setInterval(iTimeout);
    QEventLoop loop;
    QObject::connect(&t, &QTimer::timeout, &loop, [&loop, &bTimeout]() {
      bTimeout = true;
      loop.quit();
    });
    QObject::connect(sender, signal, &loop, &QEventLoop::quit);
    loop.exec();
    return !bTimeout;
  }
}

//----------------------------------------------------------------------------------------
//
QButtplugClientPrivate::QButtplugClientPrivate(QButtplugClient* const q) :
  QObject(q),
  q_ptr(q)
{
  m_pMsgSerializer = new QButtplugMessageSerializer;

  using namespace std::placeholders;
  m_spontaniousMsgHandlers = {
    { "ScanningFinished", std::bind(&QButtplugClientPrivate::q_handle_scanning_finished, this, _1) },
    { "DeviceAdded", std::bind(&QButtplugClientPrivate::q_handle_device_added, this, _1) },
    { "DeviceRemoved", std::bind(&QButtplugClientPrivate::q_handle_device_removed, this, _1) },
    { "SensorReading", std::bind(&QButtplugClientPrivate::q_handle_sensor_reading, this, _1) },
    { "RawReading", std::bind(&QButtplugClientPrivate::q_handle_raw_reading, this, _1) }
  };

  m_wsThread.setObjectName("ButtplugWebsocketThread");
  QObject::connect(&m_wsThread, &QThread::started, this, [this]() mutable {
    m_pWs = new QWebSocket();

    QObject::connect(m_pWs, &QWebSocket::connected,
                     this, &QButtplugClientPrivate::startHandshake, Qt::DirectConnection);
    QObject::connect(m_pWs, &QWebSocket::disconnected, this, &QButtplugClientPrivate::onDisconnect);
    QObject::connect(m_pWs, &QWebSocket::textMessageReceived,
                     this, &QButtplugClientPrivate::textMessageRecieved, Qt::QueuedConnection);
    QObject::connect(m_pWs, qOverload<QAbstractSocket::SocketError>(&QWebSocket::error),
                     this, &QButtplugClientPrivate::socketError, Qt::DirectConnection);

  }, Qt::DirectConnection);
  QObject::connect(&m_wsThread, &QThread::finished, this, [this]() mutable {
    m_pWs->abort();
    delete m_pWs;
  }, Qt::DirectConnection);
  m_wsThread.start();

  QObject::connect(&m_pingTimer, &QTimer::timeout, this, &QButtplugClientPrivate::pingTimerTimeout);
}

QButtplugClientPrivate::~QButtplugClientPrivate()
{
  if (m_pingTimer.isActive())
    m_pingTimer.stop();

  q_clearQueue();

  for (auto it = m_devices.begin(); m_devices.end() != it; ++it) {
    it->reset();
  }
  m_devices.clear();

  if (m_wsThread.isRunning()) {
    m_wsThread.quit();
    m_wsThread.wait();
  }
  delete m_pMsgSerializer;
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClientPrivate::waitConnected(qint64 iTimeoutMs)
{
  if (QtButtplug::Connected == m_connState)
    return true;
  return q_busy_wait(this, &QButtplugClientPrivate::connected, iTimeoutMs);
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClientPrivate::waitDisconnected(qint64 iTimeoutMs)
{
  if (QtButtplug::Disconnected == m_connState)
    return true;
  return q_busy_wait(this, &QButtplugClientPrivate::disconnected, iTimeoutMs);
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::connectToHost(const QHostAddress& sAddr, qint16 iPort)
{
  q_resetErr();

  m_hostAddr = sAddr;
  m_iPort = iPort;

  if (QtButtplug::AnyProtocolVersion == m_iMsgVersionUsed) {
    m_bRunningMsgDiscovery = true;
    m_iMsgVersionUsed = QtButtplug::ProtocolV3;
    m_pMsgSerializer->setProtocolVersion(m_iMsgVersionUsed);
  }

  //"ws://<QHostAddress>:<iPort>/buttplug"
  QUrl url;
  url.setScheme("ws");
  url.setHost(m_hostAddr.toString());
  url.setPort(m_iPort);
  url.setPath("/buttplug");
  bool bOk = qt_callInThread(&m_wsThread, [url, this]() {
    m_pWs->open(url);
  });

  if (Q_UNLIKELY(!bOk)) {
    m_bRunningMsgDiscovery = false;
    m_connState = QtButtplug::ConnectionState::Disconnected;
    return;
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::disconnectFromHost()
{
  if (QtButtplug::Disconnected == m_connState)
    return;

  q_clearQueue();

  bool bOk = qt_callInThread(&m_wsThread, [this]() {
    m_pWs->close();
  });

  if (Q_UNLIKELY(!bOk)) {
    q_setErr(QtButtplug::ERROR_UNKNOWN, QString());
    m_pWs->abort();
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::startScan()
{
  if (QtButtplug::Connected != m_connState)
    return;

  ButtplugStartScanning* scan = new ButtplugStartScanning;
  scan->Id = getNextId();

  send(scan, QStringList() << "Ok" << "Error",
       std::bind(&QButtplugClientPrivate::q_handle_scanResponse, this,
                 std::placeholders::_1, true));
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::stopScan()
{
  if (QtButtplug::Connected != m_connState)
    return;

  ButtplugStopScanning* scan = new ButtplugStopScanning;
  scan->Id = getNextId();

  send(scan, QStringList() << "Ok" << "Error",
       std::bind(&QButtplugClientPrivate::q_handle_scanResponse, this,
                 std::placeholders::_1, false));
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientPrivate::stopAllDevices()
{
  if (QtButtplug::ConnectionState::Connected != m_connState)
    return QtButtplug::Error::ERROR_SOCKET_ERR;

  ButtplugStopAllDevices* stop = new ButtplugStopAllDevices;
  stop->Id = getNextId();

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  send(stop, QStringList() << "Ok" << "Error",
       [this, &err, sError] (ButtplugMessageBase* pMsg) mutable {
    if ("Error" == pMsg->MessageType) {
      auto pErr = dynamic_cast<ButtplugError*>(pMsg);
      err = pErr->ErrorCode;
      sError = pErr->ErrorMessage;
    }
    emit q_await_response();
  });

  if (!q_busy_wait(this, &QButtplugClientPrivate::q_await_response, c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::onDisconnect()
{
  m_wsThread.setObjectName("ButtplugWebsocketThread");
  m_connState = QtButtplug::ConnectionState::Disconnected;
  if (m_pingTimer.isActive())
    m_pingTimer.stop();
  emit disconnected();
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::pingTimerTimeout()
{
  ButtplugPing* ping =  new ButtplugPing;
  ping->Id = getNextId();
  send(ping, QStringList() << "Ok" << "Error");
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::socketError(QAbstractSocket::SocketError error)
{
  switch(error) {
    case QAbstractSocket::HostNotFoundError:
    case QAbstractSocket::ConnectionRefusedError:
      qt_callInThread(this->thread(), [this]() {
        onDisconnect();
      });
      break;
    default: break;
  }

  q_setErr(QtButtplug::ERROR_SOCKET_ERR, m_pWs->errorString());
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::startHandshake()
{
  m_connState = QtButtplug::ConnectionState::Handshake;

  ButtplugRequestServerInfo* info = new ButtplugRequestServerInfo;
  info->Id = getNextId();
  info->ClientName = m_sClientName;
  info->MessageVersion = m_iMsgVersionUsed;

  if (!send(info, QStringList() << "ServerInfo" << "Error",
            std::bind(&QButtplugClientPrivate::q_handle_handshake_response, this, std::placeholders::_1))) {
    m_bRunningMsgDiscovery = false;
    m_pWs->abort();
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::textMessageRecieved(const QString& sMessage)
{
  QList<ButtplugMessageBase*> vpMsgs = m_pMsgSerializer->Deserialize(sMessage);
  for (ButtplugMessageBase* pMsg : qAsConst(vpMsgs)) {

    if (c_iSpontaniousMsgId == pMsg->Id) {
      // handle spontanious messages
      auto it = m_spontaniousMsgHandlers.find(pMsg->MessageType);
      if (m_spontaniousMsgHandlers.end() != it) {
        it->second(pMsg);
      }
    }

    else {
      // handle responses
      for (auto it = m_clientPackageQueue.begin(); m_clientPackageQueue.end() != it; ++it) {
        if (it->m_iId == pMsg->Id) {
          bool bOk = true;
          if (!it->m_vsExpectedResponses.contains(pMsg->MessageType)) {
            bOk = false;
            qWarning() << tr("Incompatible response to buttplug message:") << pMsg->Id;
          }

          if (it->m_fnResponseHandler && bOk) {
            it->m_fnResponseHandler(pMsg);
          }

          it = m_clientPackageQueue.erase(it);
          if (!m_clientPackageQueue.empty())
            --it;
          else
            break;
        }
      }
    }

    // cleanup
    delete pMsg;
  }
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClientPrivate::send(ButtplugMessageBase* pMsg, QStringList vsExpectedResponses,
                                  std::function<void(ButtplugMessageBase*)> fnRespHandler)
{
  m_clientPackageQueue.push_back({pMsg->Id, pMsg, vsExpectedResponses, fnRespHandler});

  const QString sStr = m_pMsgSerializer->Serialize(pMsg);
  bool bOk = qt_callInThread(&m_wsThread, [this, sStr]() {
    qint64 iSent = m_pWs->sendTextMessage(sStr);
    Q_UNUSED(iSent)
  });

  if (Q_UNLIKELY(!bOk)) {
    q_setErr(QtButtplug::ERROR_UNKNOWN, QString());
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
qint64 QButtplugClientPrivate::getNextId()
{
  qint64 iId = m_iLastId;
  if (std::numeric_limits<qint64>::max() == m_iLastId)
    m_iLastId = c_iSpontaniousMsgId+1;
  else
    m_iLastId++;
  return iId;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_handle_scanResponse(ButtplugMessageBase* pMsg, bool bStart)
{
  if (pMsg->MessageType == "Error") {
    auto pMsgErr = dynamic_cast<ButtplugError*>(pMsg);
    q_setErr(pMsgErr->ErrorCode, pMsgErr->ErrorMessage);
  }

  q_resetErr();

  m_bIsScanning = bStart;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_handle_handshake_response(ButtplugMessageBase* pMsg)
{
  if (pMsg->MessageType == "Error") {
    auto pMsgErr = dynamic_cast<ButtplugError*>(pMsg);
    if (m_bRunningMsgDiscovery &&
        QtButtplug::AnyProtocolVersion == m_iMsgVersionSupported) {
      if (QtButtplug::ProtocolV0 >= m_iMsgVersionUsed) {
        // retry with lower protocol version
        m_iMsgVersionUsed =
            QtButtplug::ButtplugProtocolVersion(static_cast<qint32>(m_iMsgVersionUsed) - 1);
        startHandshake();
        return;
      }

      q_setErr(pMsgErr->ErrorCode, pMsgErr->ErrorMessage);
    } else {
      q_setErr(pMsgErr->ErrorCode, pMsgErr->ErrorMessage);
    }
    return;
  }

  q_resetErr();

  if (pMsg->MessageType == "ServerInfo") {
    auto pMsgInfo = dynamic_cast<ButtplugServerInfo*>(pMsg);
    m_sServerName = pMsgInfo->ServerName;
    m_iMaxPingTime = std::max(c_iMinPingTimeMs, pMsgInfo->MaxPingTime);

    m_wsThread.setObjectName("ButtplugWebsocketThread: " + m_sServerName);

    m_connState = QtButtplug::ConnectionState::Connected;
    m_bRunningMsgDiscovery = false;
    emit connected();

    qDebug() << tr("Connected to buttplug server: ") + m_sServerName;

    m_pingTimer.setSingleShot(false);
    m_pingTimer.start(m_iMaxPingTime / 2);

    // can't modify local message queue while working it so queue the device querry instead
    qt_callInThread(this->thread(), [this]() {
      ButtplugRequestDeviceList* pDevs = new ButtplugRequestDeviceList;
      pDevs->Id = getNextId();
      send(pDevs, QStringList() << "DeviceList" << "Error",
           std::bind(&QButtplugClientPrivate::q_handle_device_list, this, std::placeholders::_1));
    });
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_handle_scanning_finished(ButtplugMessageBase*)
{
  emit scanningFinished();
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_handle_device_list(ButtplugMessageBase* pMsg)
{
  auto pMsgDevs = dynamic_cast<ButtplugDeviceList*>(pMsg);
  for (const ButtplugDevice& dev : qAsConst(pMsgDevs->Devices)) {
    if (m_devices.end() == m_devices.find(dev.DeviceIndex)) {
      QButtplugClientDevice device(this, &dev);
      m_devices.insert(dev.DeviceIndex, device);
      emit deviceAdded(dev.DeviceIndex, device);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_handle_device_added(ButtplugMessageBase* pMsg)
{
  auto pMsgAdd = dynamic_cast<ButtplugDeviceAdded*>(pMsg);

  if (m_devices.end() == m_devices.find(pMsgAdd->DeviceIndex)) {
    QButtplugClientDevice dev(this, pMsgAdd);
    m_devices.insert(pMsgAdd->DeviceIndex, dev);
    emit deviceAdded(pMsgAdd->DeviceIndex, dev);
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_handle_device_removed(ButtplugMessageBase* pMsg)
{
  auto pMsgRem = dynamic_cast<ButtplugDeviceRemoved*>(pMsg);
  auto it = m_devices.find(pMsgRem->DeviceIndex);
  if (m_devices.end() != it) {
    it->reset();
    m_devices.erase(it);
    emit deviceRemoved(pMsgRem->DeviceIndex);
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_handle_sensor_reading(ButtplugMessageBase* pMsg)
{
  // TODO:
  auto pMsgRead = dynamic_cast<ButtplugSensorReading*>(pMsg);
  Q_UNUSED(pMsgRead)
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_handle_raw_reading(ButtplugMessageBase* pMsg)
{
  // TODO:
  auto pMsgRead = dynamic_cast<ButtplugRawReading*>(pMsg);
  Q_UNUSED(pMsgRead)
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_clearQueue()
{
  while (!m_clientPackageQueue.empty()) {
    auto pFront = m_clientPackageQueue.front();
    delete pFront.m_pOutMsg;
    m_clientPackageQueue.erase(m_clientPackageQueue.begin());
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_setErr(QtButtplug::Error err, const QString& sErr)
{
  QMutexLocker l(&m_errMut);
  m_error = err;
  m_errorDetailString = sErr;
  emit errorRecieved(err);
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::q_resetErr()
{
  QMutexLocker l(&m_errMut);
  m_errorDetailString = QString();
  m_error = QtButtplug::ERROR_OK;
}
QT_END_NAMESPACE
