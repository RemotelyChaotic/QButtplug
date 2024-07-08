#ifndef QBUTTPLUGCLIENTDEVICE_H_P_H
#define QBUTTPLUGCLIENTDEVICE_H_P_H

#include "qbuttplugclientdevice.h"
#include "qbuttplugclient_p.h"

#include <QObject>
#include <QMap>
#include <QString>

QT_BEGIN_NAMESPACE

class QButtplugClientDevicePrivate : public QObject
{
  Q_OBJECT
  friend class QButtplugClientDevice;

public:
  QButtplugClientDevicePrivate(QButtplugClientPrivate* const pParent,
                               const ButtplugDevice* const pMsg) :
    QObject(pParent),
    m_pParent(pParent)
  {
    m_iId = pMsg->DeviceIndex;
    m_sName = pMsg->DeviceName;
    m_sDisplayName = pMsg->DeviceDisplayName;
    m_iTimingGap = pMsg->DeviceMessageTimingGap;
    m_messages = pMsg->DeviceMessages;
  }
  ~QButtplugClientDevicePrivate()
  {
  }

  double batteryLevel(QtButtplug::Error* pErr);
  double rssiLevel(QtButtplug::Error* pErr);
  QList<int> readSensor(quint32 sensorIndex, QtButtplug::Error* pErr);

  QtButtplug::Error sendStopDeviceCmd();
  QtButtplug::Error sendScalarCmd(double dValue, qint32 accuatorIndex);
  QtButtplug::Error sendLinearCmd(double dDurationMs, double dPosition, qint32 accuatorIndex);
  QtButtplug::Error sendRotateCmd(bool bClockwise, double dSpeed, qint32 accuatorIndex);
  QtButtplug::Error sendVibrateCmd(double dSpeed, qint32 accuatorIndex);

  QtButtplug::Error sendSensorSubscribeCmd(qint32 sensorIndex);
  QtButtplug::Error sendSensorUnsubscribeCmd(qint32 sensorIndex);

protected:
  QButtplugClientPrivate* m_pParent;

  quint32 m_iId = 0;
  QString m_sName;
  QString m_sDisplayName;
  qint64 m_iTimingGap;

  QMap<QString, QList<ButtplugClientDeviceMessageAttribute>> m_messages;

  QtButtplug::Error m_lastError = QtButtplug::ERROR_OK;
};

QT_END_NAMESPACE

#endif // QBUTTPLUGCLIENTDEVICE_H_P_H
