#ifndef QBUTTPLUGCLIENTDEVICE_H
#define QBUTTPLUGCLIENTDEVICE_H

#include "qbuttplug_export.h"
#include "qbuttplugenums.h"
#include "qbuttplugmessages.h"

#include <QPointer>
#include <QStringList>

#include <functional>

QT_BEGIN_NAMESPACE

class QButtplugClientPrivate;
class QButtplugClientDevicePrivate;

class QBUTTPLUG_EXPORT QButtplugClientDevice
{
public:
  QButtplugClientDevice();
  QButtplugClientDevice(const QButtplugClientDevice& other);
  ~QButtplugClientDevice();

  bool isValid() const;
  bool isNull() const;

  // ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3
  quint32 id() const;
  // ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3
  QString name() const;
  // ProtocolV3, equals name() otherwise
  QString displayName() const;
  // ProtocolV3
  qint64 timingGap() const;

  // ProtocolV2, ProtocolV3 noop else
  double batteryLevel(QtButtplug::Error* pErr = nullptr);
  // ProtocolV2, ProtocolV3 noop else
  double rssiLevel(QtButtplug::Error* pErr = nullptr);
  // ProtocolV3 noop else
  QList<int> readSensor(quint32 sensorIndex, QtButtplug::Error* pErr = nullptr);

  QStringList supportedMessages() const;
  QList<ButtplugClientDeviceMessageAttribute> messageAttributes(const QString& sMsg) const;

  QtButtplug::Error sendStopDeviceCmd();
  // mapps to sendVibrateCmd for ProtocolV0, ProtocolV1
  QtButtplug::Error sendScalarCmd(double dValue, qint32 accuatorIndex = -1);
  // noop for ProtocolV0
  QtButtplug::Error sendLinearCmd(double dDurationMs, double dPosition, qint32 accuatorIndex = -1);
  // noop for ProtocolV0
  QtButtplug::Error sendRotateCmd(bool bClockwise, double dSpeed, qint32 accuatorIndex = -1);
  // mapps to sendScalarCmd for ProtocolV2, ProtocolV3
  QtButtplug::Error sendVibrateCmd(double dSpeed, qint32 accuatorIndex = -1);

  // ProtocolV3 noop else
  QtButtplug::Error sendSensorSubscribeCmd(qint32 sensorIndex);
  // ProtocolV3 noop else
  QtButtplug::Error sendSensorUnsubscribeCmd(qint32 sensorIndex);

  void setSensorReadingCallback(std::function<void(qint32 /*iSensorIndex*/, QList<int> /*data*/)> fn);

protected:
  QButtplugClientDevice(QButtplugClientPrivate* pParent);

private:
  QButtplugClientDevicePrivate* d_func();
  const QButtplugClientDevicePrivate* d_func() const;

  friend class QButtplugClient;
  QPointer<QButtplugClientDevicePrivate> d_ptr;
  std::function<void(qint32 /*iSensorIndex*/, QList<int> /*data*/)> m_fnCallback;
};

QT_END_NAMESPACE

#endif //QBUTTPLUGCLIENTDEVICE_H
