#include "qbuttplugclient_p.h"
#include "qbuttplugclientdevice_p.h"

QT_BEGIN_NAMESPACE

//----------------------------------------------------------------------------------------
//
double QButtplugClientDevicePrivate::batteryLevel(QtButtplug::Error* pErr)
{
  Q_UNUSED(pErr)
  return 0.0;
}

//----------------------------------------------------------------------------------------
//
double QButtplugClientDevicePrivate::rssiLevel(QtButtplug::Error* pErr)
{
  Q_UNUSED(pErr)
  return 0.0;
}

//----------------------------------------------------------------------------------------
//
QList<int> QButtplugClientDevicePrivate::readSensor(quint32 sensorIndex, QtButtplug::Error* pErr)
{
  Q_UNUSED(sensorIndex)
  Q_UNUSED(pErr)
  return {};
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendStopDeviceCmd()
{
  return QtButtplug::ERROR_OK;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendScalarCmd(double dValue, qint32 accuatorIndex)
{
  Q_UNUSED(dValue)
  Q_UNUSED(accuatorIndex)
  return QtButtplug::ERROR_OK;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendLinearCmd(double dDurationMs, double dPosition, qint32 accuatorIndex)
{
  Q_UNUSED(dDurationMs)
  Q_UNUSED(dPosition)
  Q_UNUSED(accuatorIndex)
  return QtButtplug::ERROR_OK;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendRotateCmd(bool bClockwise, double dSpeed, qint32 accuatorIndex)
{
  Q_UNUSED(bClockwise)
  Q_UNUSED(dSpeed)
  Q_UNUSED(accuatorIndex)
  return QtButtplug::ERROR_OK;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendVibrateCmd(double dSpeed, qint32 accuatorIndex)
{
  Q_UNUSED(dSpeed)
  Q_UNUSED(accuatorIndex)
  return QtButtplug::ERROR_OK;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendSensorSubscribeCmd(qint32 sensorIndex)
{
  Q_UNUSED(sensorIndex)
  return QtButtplug::ERROR_OK;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendSensorUnsubscribeCmd(qint32 sensorIndex)
{
  Q_UNUSED(sensorIndex)
  return QtButtplug::ERROR_OK;
}

QT_END_NAMESPACE
