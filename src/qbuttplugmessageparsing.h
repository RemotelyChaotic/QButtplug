#ifndef QBUTTPLUGMESSAGEPARSING_H
#define QBUTTPLUGMESSAGEPARSING_H

#include "qbuttplugmessages.h"

#include <QJsonObject>

#include <functional>

namespace QtButtplug
{
  void qRegisterMssageParser(const QString& sMsg,
                             std::function<QtButtplug::MessageBase*(const QJsonObject&, QtButtplug::ButtplugProtocolVersion)> fnFromJson,
                             std::function<QJsonObject(QtButtplug::MessageBase*, QtButtplug::ButtplugProtocolVersion)> fnToJson);
}

QT_BEGIN_NAMESPACE

class QButtplugMessageSerializer
{
public:
  QButtplugMessageSerializer();

  void setProtocolVersion(QtButtplug::ButtplugProtocolVersion v);

  QString Serialize(QtButtplug::MessageBase* pMsg);
  QList<QtButtplug::MessageBase*> Deserialize(const QString& sMsg);

private:
  QtButtplug::ButtplugProtocolVersion m_v;
};

QT_END_NAMESPACE

#endif // QBUTTPLUGMESSAGEPARSING_H
