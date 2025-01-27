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
  // ProtocolV3 default otherwise
  qint64 timingGap() const;

  // ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
  double batteryLevel(QtButtplug::Error* pErr = nullptr);
  // ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
  double rssiLevel(QtButtplug::Error* pErr = nullptr);
  // ProtocolV3, ERROR_NOT_SUPPORTED else
  QList<int> readSensor(quint32 sensorIndex, QtButtplug::Error* pErr = nullptr);
  // ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
  // DO NOT USE UNLESS YOU KNOW WHAT YOU'RE DOING!!!!
  QByteArray sendRawReadCmd(const QString& endpoint, qint64 iExpectedLength,
                            bool bWaitForData, QtButtplug::Error* pErr = nullptr);

  QStringList supportedMessages() const;
  QList<QtButtplug::ClientDeviceMessageAttribute> messageAttributes(const QString& sMsg) const;

  QtButtplug::Error sendStopDeviceCmd();
  // mapps to sendVibrateCmd for ProtocolV0, ProtocolV1, ProtocolV2
  QtButtplug::Error sendScalarCmd(double dValue, qint32 accuatorIndex = -1);
  // ERROR_NOT_SUPPORTED for ProtocolV0
  QtButtplug::Error sendLinearCmd(qint64 iDurationMs, double dPosition, qint32 accuatorIndex = -1);
  // ERROR_NOT_SUPPORTED for ProtocolV0
  QtButtplug::Error sendRotateCmd(bool bClockwise, double dSpeed, qint32 accuatorIndex = -1);
  // mapps to sendScalarCmd for ProtocolV3
  QtButtplug::Error sendVibrateCmd(double dSpeed, qint32 accuatorIndex = -1);

  // supported for ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3, but the current server
  // ProtocolV0+ might not support this.
  // Use a generic command whenever possible
  Q_DECL_DEPRECATED QtButtplug::Error sendKiirooCmd(const QString& cmd);

  // supported for ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3, but the current server
  // ProtocolV0+ might not support this.
  // Use a generic command whenever possible
  // iPosition and iSpeed must be in range [0-99]
  Q_DECL_DEPRECATED QtButtplug::Error sendFleshlightLaunchFW12Cmd(quint32 iPosition, quint32 iSpeed);

  // supported for ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3, but the current server
  // ProtocolV0+ might not support this.
  // Use a generic command whenever possible
  Q_DECL_DEPRECATED QtButtplug::Error sendLovenseCmd(const QString& cmd);

  // supported for ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3, but the current server
  // ProtocolV0+ might not support this.
  // Use a generic command whenever possible
  Q_DECL_DEPRECATED QtButtplug::Error sendVorzeA10CycloneCmd(double dSpeed, bool bClockwise);

  // ProtocolV3, ERROR_NOT_SUPPORTED else
  QtButtplug::Error sendSensorSubscribeCmd(qint32 sensorIndex);
  // ProtocolV3, ERROR_NOT_SUPPORTED else
  QtButtplug::Error sendSensorUnsubscribeCmd(qint32 sensorIndex);

  // ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
  // DO NOT USE UNLESS YOU KNOW WHAT YOU'RE DOING!!!!
  QtButtplug::Error sendRawWriteCmd(const QString& endpoint, QByteArray data, bool bWriteWithResponse);
  // ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
  // DO NOT USE UNLESS YOU KNOW WHAT YOU'RE DOING!!!!
  QtButtplug::Error sendRawSubscribeCmd(const QString& endpoint);
  // ProtocolV2, ProtocolV3, ERROR_NOT_SUPPORTED else
  // DO NOT USE UNLESS YOU KNOW WHAT YOU'RE DOING!!!!
  QtButtplug::Error sendRawUnsubscribeCmd(const QString& endpoint);

  // The callback set is per instance of QButtplugClientDevice. Copying creating a new
  // instance of QButtplugClientDevice via copy-constructor does not copy the callback
  // to the new instance.
  void setSensorReadingCallback(std::function<void(qint32 /*iSensorIndex*/, QList<int> /*data*/)> fn);
  // The callback set is per instance of QButtplugClientDevice. Copying creating a new
  // instance of QButtplugClientDevice via copy-constructor does not copy the callback
  // to the new instance.
  void setRawReadingCallback(std::function<void(const QString& /*endpoint*/, const QString& /*data*/)> fn);

  QtButtplug::Error error() const;
  QString errorString() const;

protected:
  QButtplugClientDevice(QButtplugClientPrivate* pParent, const QtButtplug::Device* const pMsg);

  void reset();
  void sensorReadingReceived(QtButtplug::SensorReading* pMsg);
  void rawReadingReceived(QtButtplug::RawReading* pMsg);

private:
  QButtplugClientDevicePrivate* d_func();
  const QButtplugClientDevicePrivate* d_func() const;

  friend class QButtplugClientPrivate;
  QPointer<QButtplugClientDevicePrivate> d_ptr;
};

QT_END_NAMESPACE

#endif //QBUTTPLUGCLIENTDEVICE_H
