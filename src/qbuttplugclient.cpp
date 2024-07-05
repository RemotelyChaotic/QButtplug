#include "qbuttplugclient.h"
#include "qbuttplugclient_p.h"

QButtplugClient::QButtplugClient(QObject* pParent) :
  QObject(pParent),
  d_ptr(new QButtplugClientPrivate(this))
{

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
void QButtplugClient::connect()
{
  Q_D(QButtplugClient);
  connect(d->m_hostAddr, d->m_iPort);
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::connect(const QHostAddress& sAddr, qint16 iPort)
{
  Q_D(QButtplugClient);
  d->connect(sAddr, iPort);
}

//----------------------------------------------------------------------------------------
//
void QButtplugClient::disconnect()
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
  return false;
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClient::waitDisconnected(qint64 iTimeoutMs)
{
  return false;
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
  return d->m_error;
}

//----------------------------------------------------------------------------------------
//
QString QButtplugClient::errorString() const
{
  const Q_D(QButtplugClient);
  return errorString(d->m_error);
}

//----------------------------------------------------------------------------------------
//
QString QButtplugClient::errorString(QtButtplug::Error error)
{
  switch (error)
  {
    case QtButtplug::ERROR_OK:
      return tr("No Error.");
    case QtButtplug::ERROR_UNKNOWN:
      return tr("An unknown error occurred.");
    case QtButtplug::ERROR_INIT:
      return tr("Handshake did not succeed.");
    case QtButtplug::ERROR_PING:
      return tr("A ping was not sent in the expected time.");
    case QtButtplug::ERROR_MSG:
      return tr("A message parsing or permission error occurred.");
    case QtButtplug::ERROR_DEVICE:
      return tr("A command sent to a device returned an error.");
    case QtButtplug::ERROR_PING_TIMEOUT:
      return tr("Ping timeout.");
  }
  return tr("No Error.");
}
