#include "qbuttplugclient.h"
#include "qbuttplugclient_p.h"
#include "qbuttplugmessageparsing.h"

QButtplugClient::QButtplugClient(QObject* pParent) :
  QObject(pParent),
  d_ptr(new QButtplugClientPrivate(this))
{
  QObject::connect(d_ptr, &QButtplugClientPrivate::connected, this, &QButtplugClient::connected);
  QObject::connect(d_ptr, &QButtplugClientPrivate::disconnected, this, &QButtplugClient::disconnected);
  QObject::connect(d_ptr, &QButtplugClientPrivate::deviceAdded, this, &QButtplugClient::deviceAdded);
  QObject::connect(d_ptr, &QButtplugClientPrivate::deviceRemoved, this, &QButtplugClient::deviceRemoved);
  QObject::connect(d_ptr, &QButtplugClientPrivate::scanningFinished, this, &QButtplugClient::scanningFinished);
  QObject::connect(d_ptr, &QButtplugClientPrivate::errorRecieved, this, &QButtplugClient::errorRecieved);
}
QButtplugClient::~QButtplugClient()
{

}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::setClientName(const QString& sClientName)
{
  Q_D(QButtplugClient);
  d->m_sClientName = sClientName;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::setAddress(const QHostAddress& sAddr)
{
  Q_D(QButtplugClient);
  d->m_hostAddr = sAddr;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::setPort(qint16 iPort)
{
  Q_D(QButtplugClient);
  d->m_iPort = iPort;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::setProtocolVersion(QtButtplug::ButtplugProtocolVersion iVersion)
{
  Q_D(QButtplugClient);
  d->m_iMsgVersionSupported = iVersion;
}

//----------------------------------------------------------------------------------------
//
QString QButtplugClient::clientName() const
{
  const Q_D(QButtplugClient);
  if (d->m_sClientName.isEmpty())
    return objectName();
  return d->m_sClientName;
}

//----------------------------------------------------------------------------------------
//
QString QButtplugClient::serverName() const
{
  const Q_D(QButtplugClient);
  return d->m_sServerName;
}

//----------------------------------------------------------------------------------------
//
QHostAddress QButtplugClient::address() const
{
  const Q_D(QButtplugClient);
  return d->m_hostAddr;
}

//----------------------------------------------------------------------------------------
//
qint16 QButtplugClient::port() const
{
  const Q_D(QButtplugClient);
  return d->m_iPort;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::ButtplugProtocolVersion QButtplugClient::messageVersionUsed() const
{
  const Q_D(QButtplugClient);
  return d->m_iMsgVersionUsed;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::ButtplugProtocolVersion QButtplugClient::messageVersionSupported() const
{
  const Q_D(QButtplugClient);
  return d->m_iMsgVersionSupported;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::ConnectionState QButtplugClient::connectionState() const
{
  const Q_D(QButtplugClient);
  return d->m_connState;
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClient::isScanning() const
{
  const Q_D(QButtplugClient);
  return d->m_bIsScanning;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::connectToHost()
{
  Q_D(QButtplugClient);
  connectToHost(d->m_hostAddr, d->m_iPort);
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::connectToHost(const QHostAddress& sAddr, qint16 iPort)
{
  Q_D(QButtplugClient);
  if (QtButtplug::ConnectionState::Disconnected != d->m_connState)
    return;

  d->m_connState = QtButtplug::ConnectionState::Connecting;
  d->m_iMsgVersionUsed = d->m_iMsgVersionSupported;
  d->m_pMsgSerializer->setProtocolVersion(d->m_iMsgVersionUsed);
  d->connectToHost(sAddr, iPort);
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::disconnectFromHost()
{
  Q_D(QButtplugClient);
  d->disconnect();
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::startScan()
{
  Q_D(QButtplugClient);
  d->startScan();
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::stopScan()
{
  Q_D(QButtplugClient);
  d->stopScan();
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClient::waitConnected(qint64 iTimeoutMs)
{
  Q_D(QButtplugClient);
  return d->waitConnected(iTimeoutMs);
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClient::waitDisconnected(qint64 iTimeoutMs)
{
  Q_D(QButtplugClient);
  return d->waitDisconnected(iTimeoutMs);
}

//----------------------------------------------------------------------------------------
//
qint32 QButtplugClient::deviceCount() const
{
  const Q_D(QButtplugClient);
  return d->m_devices.count();
}

//----------------------------------------------------------------------------------------
//
QButtplugClientDevice QButtplugClient::device(quint32 iId) const
{
  const Q_D(QButtplugClient);
  auto it = d->m_devices.find(iId);
  if (d->m_devices.end() != it)
  {
    return it.value();
  }
  return QButtplugClientDevice();
}

//----------------------------------------------------------------------------------------
//
QList<QButtplugClientDevice> QButtplugClient::devices() const
{
  const Q_D(QButtplugClient);
  QList<QButtplugClientDevice> vOut;
  for (auto it = d->m_devices.begin(); d->m_devices.end() != it; ++it)
  {
    vOut << it.value();
  }
  return vOut;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClient::stopAllDevices()
{
  Q_D(QButtplugClient);
  return d->stopAllDevices();
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClient::error() const
{
  const Q_D(QButtplugClient);
  QMutexLocker l(&d->m_errMut);
  return d->m_error;
}

//----------------------------------------------------------------------------------------
//
QString QButtplugClient::errorString() const
{
  const Q_D(QButtplugClient);
  QMutexLocker l(&d->m_errMut);
  return qt_errorString(d->m_error, d->m_errorDetailString);
}

//----------------------------------------------------------------------------------------
//
QString QButtplugClient::errorString(QtButtplug::Error error)
{
  return qt_errorString(error, QString());
}
