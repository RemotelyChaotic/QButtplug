#include "qbuttplugclientdevice.h"
#include "qbuttplugclientdevice_p.h"

QButtplugClientDevice::QButtplugClientDevice() :
  d_ptr(nullptr)
{
}

QButtplugClientDevice::QButtplugClientDevice(QButtplugClientPrivate* pParent) :
  d_ptr(new QButtplugClientDevicePrivate(pParent))
{
}

QButtplugClientDevice::QButtplugClientDevice(const QButtplugClientDevice& other) :
  d_ptr(other.d_ptr)
{
}

QButtplugClientDevice::~QButtplugClientDevice()
{
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClientDevice::isValid() const
{
  return nullptr == d_ptr;
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClientDevice::isNull() const
{
  return nullptr == d_ptr;
}

//----------------------------------------------------------------------------------------
//
// ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3
quint32 QButtplugClientDevice::id() const
{
  if (!isValid())
    return 0;

  const Q_D(QButtplugClientDevice);
  return d->m_iId;
}

//----------------------------------------------------------------------------------------
//
// ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3
QString QButtplugClientDevice::name() const
{
  if (!isValid())
    return QString();

  const Q_D(QButtplugClientDevice);
  return d->m_sName;
}

//----------------------------------------------------------------------------------------
//
// ProtocolV3, equals name() otherwise
QString QButtplugClientDevice::displayName() const
{
  if (!isValid())
    return QString();

  const Q_D(QButtplugClientDevice);
  return d->m_sDisplayName;
}

//----------------------------------------------------------------------------------------
//
// ProtocolV3
qint64 QButtplugClientDevice::timingGap() const
{
  if (!isValid())
    return -1;

  const Q_D(QButtplugClientDevice);
  return d->m_iTimingGap;
}

//----------------------------------------------------------------------------------------
//
// ProtocolV2, ProtocolV3 noop else
double QButtplugClientDevice::batteryLevel(QtButtplug::Error* pErr)
{
  if (!isValid()) {
    if (nullptr != pErr)
      *pErr = QtButtplug::ERROR_UNKNOWN;
    return 0.0;
  }

  Q_D(QButtplugClientDevice);
  return d->batteryLevel(pErr);
}

//----------------------------------------------------------------------------------------
//
// ProtocolV2, ProtocolV3 noop else
double QButtplugClientDevice::rssiLevel(QtButtplug::Error* pErr)
{
  if (!isValid()) {
    if (nullptr != pErr)
      *pErr = QtButtplug::ERROR_UNKNOWN;
    return 0.0;
  }

  Q_D(QButtplugClientDevice);
  return d->rssiLevel(pErr);
}

//----------------------------------------------------------------------------------------
//
// ProtocolV3 noop else
QList<int> QButtplugClientDevice::readSensor(quint32 sensorIndex, QtButtplug::Error* pErr)
{
  if (!isValid()) {
    if (nullptr != pErr)
      *pErr = QtButtplug::ERROR_UNKNOWN;
    return {};
  }

  Q_D(QButtplugClientDevice);
  return d->readSensor(sensorIndex, pErr);
}

//----------------------------------------------------------------------------------------
//
QStringList QButtplugClientDevice::supportedMessages() const
{
  if (!isValid())
    return QStringList();

  const Q_D(QButtplugClientDevice);
  QStringList vsRet;
  for (auto it = d->m_messages.begin(); d->m_messages.end() != it; ++it) {
    vsRet << it.key();
  }
  return vsRet;
}

//----------------------------------------------------------------------------------------
//
QList<ButtplugClientDeviceMessageAttribute> QButtplugClientDevice::messageAttributes(const QString& sMsg) const
{
  if (!isValid())
    return {};

  const Q_D(QButtplugClientDevice);
  auto it = d->m_messages.find(sMsg);
  if (d->m_messages.end() != it) {
    return it.value();
  }
  return {};
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevice::sendStopDeviceCmd()
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendStopDeviceCmd();
}

//----------------------------------------------------------------------------------------
//
// mapps to sendVibrateCmd for ProtocolV0, ProtocolV1
QtButtplug::Error QButtplugClientDevice::sendScalarCmd(double dValue, qint32 accuatorIndex)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendScalarCmd(dValue, accuatorIndex);
}

//----------------------------------------------------------------------------------------
//
// noop for ProtocolV0
QtButtplug::Error QButtplugClientDevice::sendLinearCmd(double dDurationMs, double dPosition, qint32 accuatorIndex)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendLinearCmd(dDurationMs, dPosition, accuatorIndex);
}

//----------------------------------------------------------------------------------------
//
// noop for ProtocolV0
QtButtplug::Error QButtplugClientDevice::sendRotateCmd(bool bClockwise, double dSpeed, qint32 accuatorIndex)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendRotateCmd(bClockwise, dSpeed, accuatorIndex);
}

//----------------------------------------------------------------------------------------
//
// mapps to sendScalarCmd for ProtocolV2, ProtocolV3
QtButtplug::Error QButtplugClientDevice::sendVibrateCmd(double dSpeed, qint32 accuatorIndex)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendVibrateCmd(dSpeed, accuatorIndex);
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevice::sendSensorSubscribeCmd(qint32 sensorIndex)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendSensorUnsubscribeCmd(sensorIndex);
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevice::sendSensorUnsubscribeCmd(qint32 sensorIndex)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendSensorUnsubscribeCmd(sensorIndex);
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientDevice::setSensorReadingCallback(std::function<void(qint32 /*iSensorIndex*/, QList<int> /*data*/)> fn)
{
  m_fnCallback = fn;
}

//----------------------------------------------------------------------------------------
//
QButtplugClientDevicePrivate* QButtplugClientDevice::d_func()
{
  return d_ptr;
}
const QButtplugClientDevicePrivate* QButtplugClientDevice::d_func() const
{
  return d_ptr;
}
