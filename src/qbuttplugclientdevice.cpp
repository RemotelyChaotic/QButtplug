#include "qbuttplugclientdevice.h"
#include "qbuttplugclientdevice_p.h"

QButtplugClientDevice::QButtplugClientDevice() :
  d_ptr(nullptr)
{
}

QButtplugClientDevice::QButtplugClientDevice(QButtplugClientPrivate* pParent,
                                             const QtButtplug::Device* const pMsg) :
  d_ptr(new QButtplugClientDevicePrivate(pParent, pMsg))
{
}

QButtplugClientDevice::QButtplugClientDevice(const QButtplugClientDevice& other) :
  d_ptr(other.d_ptr)
{
}

QButtplugClientDevice::~QButtplugClientDevice()
{
  if (isValid()) {
    Q_D(QButtplugClientDevice);
    auto itSens = d->m_vSensorCallbacks.find(reinterpret_cast<qint64>(this));
    if (d->m_vSensorCallbacks.end() != itSens) {
      d->m_vSensorCallbacks.erase(itSens);
    }
    auto itRead = d->m_vRawReadCallbacks.find(reinterpret_cast<qint64>(this));
    if (d->m_vRawReadCallbacks.end() != itRead) {
      d->m_vRawReadCallbacks.erase(itRead);
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClientDevice::isValid() const
{
  return !isNull();
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
// ProtocolV3 default otherwise
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
// ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
QByteArray QButtplugClientDevice::sendRawReadCmd(const QString& endpoint, qint64 iExpectedLength,
                                                 bool bWaitForData, QtButtplug::Error* pErr)
{
  if (!isValid()) {
    if (nullptr != pErr)
      *pErr = QtButtplug::ERROR_UNKNOWN;
    return {};
  }

  Q_D(QButtplugClientDevice);
  return d->sendRawReadCmd(endpoint, iExpectedLength, bWaitForData, pErr);
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
QList<QtButtplug::ClientDeviceMessageAttribute> QButtplugClientDevice::messageAttributes(const QString& sMsg) const
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
QtButtplug::Error QButtplugClientDevice::sendLinearCmd(qint64 dDurationMs, double dPosition, qint32 accuatorIndex)
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
QtButtplug::Error QButtplugClientDevice::sendKiirooCmd(const QString& cmd)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendKiirooCmd(cmd);
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevice::sendFleshlightLaunchFW12Cmd(quint32 iPosition, quint32 iSpeed)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendFleshlightLaunchFW12Cmd(iPosition, iSpeed);
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevice::sendLovenseCmd(const QString& cmd)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendLovenseCmd(cmd);
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevice::sendVorzeA10CycloneCmd(double dSpeed, bool bClockwise)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendVorzeA10CycloneCmd(dSpeed, bClockwise);
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
// ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
QtButtplug::Error QButtplugClientDevice::sendRawWriteCmd(const QString& endpoint,
                                                         QByteArray data,
                                                         bool bWriteWithResponse)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendRawWriteCmd(endpoint, data, bWriteWithResponse);
}

//----------------------------------------------------------------------------------------
//
// ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
QtButtplug::Error QButtplugClientDevice::sendRawSubscribeCmd(const QString& endpoint)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendRawSubscribeCmd(endpoint);
}

//----------------------------------------------------------------------------------------
//
// ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
QtButtplug::Error QButtplugClientDevice::sendRawUnsubscribeCmd(const QString& endpoint)
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  Q_D(QButtplugClientDevice);
  return d->sendRawUnsubscribeCmd(endpoint);
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientDevice::setSensorReadingCallback(std::function<void(qint32 /*iSensorIndex*/, QList<int> /*data*/)> fn)
{
  if (!isValid())
    return;

  Q_D(QButtplugClientDevice);
  d->m_vSensorCallbacks[reinterpret_cast<qint64>(this)] = fn;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientDevice::setRawReadingCallback(std::function<void(const QString& /*endpoint*/, const QString& /*data*/)> fn)
{
  if (!isValid())
    return;

  Q_D(QButtplugClientDevice);
  d->m_vRawReadCallbacks[reinterpret_cast<qint64>(this)] = fn;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevice::error() const
{
  if (!isValid())
    return QtButtplug::ERROR_UNKNOWN;

  const Q_D(QButtplugClientDevice);
  return d->m_lastError;
}

//----------------------------------------------------------------------------------------
//
QString QButtplugClientDevice::errorString() const
{
  if (!isValid())
    return QString();

  const Q_D(QButtplugClientDevice);
  return qt_errorString(d->m_lastError, d->m_errorDetailString);
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientDevice::reset()
{
  if (isValid()) {
    Q_D(QButtplugClientDevice);
    auto itSens = d->m_vSensorCallbacks.find(reinterpret_cast<qint64>(this));
    if (d->m_vSensorCallbacks.end() != itSens) {
      d->m_vSensorCallbacks.erase(itSens);
    }
    auto itRead = d->m_vRawReadCallbacks.find(reinterpret_cast<qint64>(this));
    if (d->m_vRawReadCallbacks.end() != itRead) {
      d->m_vRawReadCallbacks.erase(itRead);
    }
    delete d_ptr;
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientDevice::sensorReadingRecieved(QtButtplug::SensorReading* pMsg)
{
  if (!isValid())
    return;

  const Q_D(QButtplugClientDevice);
  for (auto it = d->m_vSensorCallbacks.begin(); d->m_vSensorCallbacks.end() != it; ++it) {
    it->second(pMsg->SensorIndex, pMsg->Data);
  }
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientDevice::rawReadingRecieved(QtButtplug::RawReading* pMsg)
{
  if (!isValid())
    return;

  const Q_D(QButtplugClientDevice);
  for (auto it = d->m_vRawReadCallbacks.begin(); d->m_vRawReadCallbacks.end() != it; ++it) {
    it->second(pMsg->Endpoint, pMsg->Data);
  }
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
