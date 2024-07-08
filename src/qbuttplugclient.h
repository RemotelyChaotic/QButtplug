#ifndef QBUTTPLUGCLIENT_H
#define QBUTTPLUGCLIENT_H

#include "qbuttplug_export.h"
#include "qbuttplugclientdevice.h"

#include <QFlags>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QObject>

QT_BEGIN_NAMESPACE

class QButtplugClientPrivate;

class QBUTTPLUG_EXPORT QButtplugClient : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(QButtplugClient)

public:
  explicit QButtplugClient(QObject* pParent = nullptr);
  ~QButtplugClient() override;

  void setClientName(const QString& sClientName);
  void setAddress(const QHostAddress& sAddr);
  void setPort(qint16 iPort);
  void setProtocolVersion(QtButtplug::ButtplugProtocolVersion iVersion = QtButtplug::AnyProtocolVersion);

  QString clientName() const;
  QString serverName() const;
  QHostAddress address() const;
  qint16 port() const;
  QtButtplug::ButtplugProtocolVersion messageVersionUsed() const;
  QtButtplug::ButtplugProtocolVersion messageVersionSupported() const;
  QtButtplug::ConnectionState connectionState() const;
  bool isScanning() const;

  bool waitConnected(qint64 iTimeoutMs = 30000);
  bool waitDisconnected(qint64 iTimeoutMs = 30000);

  qint32 deviceCount() const;
  QButtplugClientDevice device(quint32 iId) const;
  QList<QButtplugClientDevice> devices() const;

  QtButtplug::Error stopAllDevices();

  QtButtplug::Error error() const;
  QString errorString() const;
  static QString errorString(QtButtplug::Error error);

public slots:
  void connectToHost();
  void connectToHost(const QHostAddress& sAddr, qint16 iPort = 12345);
  void disconnectFromHost();
  void startScan();
  void stopScan();

signals:
  void connected();
  void connectionStateChanged(QtButtplug::ConnectionState state);
  void disconnected();
  void deviceAdded(quint32 iId, QButtplugClientDevice device);
  void deviceRemoved(quint32 iId);
  void scanningFinished();
  void errorRecieved(QtButtplug::Error error);

private:
  Q_DECLARE_PRIVATE(QButtplugClient)
  QButtplugClientPrivate* const d_ptr;
};

QT_END_NAMESPACE

#endif //QBUTTPLUGCLIENT_H
