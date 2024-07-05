#ifndef QBUTTPLUGMESSAGEPARSING_H
#define QBUTTPLUGMESSAGEPARSING_H

#include "qbuttplugmessages.h"

#include <QJsonObject>

#include <functional>

namespace QtButtplug
{
  void qRegisterMssageParser(const QString& sMsg,
                             std::function<ButtplugMessageBase*(const QJsonObject&, QtButtplug::ButtplugProtocolVersion)> fnFromJson,
                             std::function<QJsonObject(ButtplugMessageBase*, QtButtplug::ButtplugProtocolVersion)> fnToJson);
}

#endif // QBUTTPLUGMESSAGEPARSING_H
