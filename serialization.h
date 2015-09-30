#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <QtQml/QJSValue>

#include "qpm.h"

class QQmlEngine;

QPM_BEGIN_NAMESPACE(com, cutehacks, duperagent)

class BodyCodec
{
public:
    virtual QByteArray stringify(const QJSValue&) = 0;
    virtual QJSValue parse(const QByteArray&) = 0;

protected:
    BodyCodec(QQmlEngine *);
    QQmlEngine *m_engine;
};

class JsonCodec : public BodyCodec
{
public:
    JsonCodec(QQmlEngine *);
    QByteArray stringify(const QJSValue &);
    QJSValue parse(const QByteArray &);

protected:
    QJSValue parseJsonDocument(const QJsonDocument &);
    QJSValue parseJsonArray(const QJsonArray &);
    QJSValue parseJsonObject(const QJsonObject &);
    QJSValue parseJsonValue(const QJsonValue &);

    QJsonObject stringifyObject(const QJSValue &) const;
    QJsonArray stringifyArray(const QJSValue &) const;
    QJsonValue stringifyValue(const QJSValue &) const;
};

QPM_END_NAMESPACE(com, cutehacks, duperagent)

#endif // SERIALIZATION_H
