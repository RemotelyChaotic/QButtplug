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
                               const QtButtplug::Device* const pMsg);
  ~QButtplugClientDevicePrivate();

  double batteryLevel(QtButtplug::Error* pErr);
  double rssiLevel(QtButtplug::Error* pErr);
  QList<int> readSensor(quint32 sensorIndex, QtButtplug::Error* pErr);
  QByteArray sendRawReadCmd(const QString& endpoint, qint64 iExpectedLength,
                            bool bWaitForData, QtButtplug::Error* pErr);

  QtButtplug::Error sendStopDeviceCmd();
  QtButtplug::Error sendScalarCmd(double dValue, qint32 accuatorIndex);
  QtButtplug::Error sendLinearCmd(qint64 iDurationMs, double dPosition, qint32 accuatorIndex);
  QtButtplug::Error sendRotateCmd(bool bClockwise, double dSpeed, qint32 accuatorIndex);
  QtButtplug::Error sendVibrateCmd(double dSpeed, qint32 accuatorIndex);

  QtButtplug::Error sendKiirooCmd(const QString& cmd);
  QtButtplug::Error sendFleshlightLaunchFW12Cmd(quint32 iPosition, quint32 iSpeed);
  QtButtplug::Error sendLovenseCmd(const QString& cmd);
  QtButtplug::Error sendVorzeA10CycloneCmd(quint32 iSpeed, bool bClockwise);

  QtButtplug::Error sendSensorSubscribeCmd(qint32 sensorIndex);
  QtButtplug::Error sendSensorUnsubscribeCmd(qint32 sensorIndex);

  QtButtplug::Error sendRawWriteCmd(const QString& endpoint, QByteArray data, bool bWriteWithResponse);
  QtButtplug::Error sendRawSubscribeCmd(const QString& endpoint);
  QtButtplug::Error sendRawUnsubscribeCmd(const QString& endpoint);

protected:
  QButtplugClientPrivate* m_pParent;

  quint32 m_iId = 0;
  QString m_sName;
  QString m_sDisplayName;
  qint64 m_iTimingGap;

  QMap<QString, QList<QtButtplug::ClientDeviceMessageAttribute>> m_messages;

  std::map<qint64, std::function<void(qint32 /*iSensorIndex*/, QList<int> /*data*/)>> m_vSensorCallbacks;
  std::map<qint64, std::function<void(const QString& /*endpoint*/, const QString& /*data*/)>> m_vRawReadCallbacks;

  QtButtplug::Error m_lastError = QtButtplug::ERROR_OK;
  QString           m_errorDetailString;

private:
  QtButtplug::Error q_sendScalarCmd(double dValue, qint32 accuatorIndex);
  QtButtplug::Error q_sendVibrateCmd(double dValue, qint32 accuatorIndex);
  void q_setErr(QtButtplug::Error err, const QString& sErr);
  void q_resetErr();
};

QT_END_NAMESPACE

#endif // QBUTTPLUGCLIENTDEVICE_H_P_H
