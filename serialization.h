#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <QtQml/QJSValue>

#include "qpm.h"

class QQmlEngine;

namespace com { namespace cutehacks { namespace duperagent {

typedef QList<QPair<QString, QString> > QueryItems;

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

class FormUrlEncodedCodec : public BodyCodec
{
public:
    FormUrlEncodedCodec(QQmlEngine *);

public:
    QByteArray stringify(const QJSValue &);
    QJSValue parse(const QByteArray &);

protected:
    QueryItems stringifyObject(const QString&, const QJSValue &) const;
    QueryItems stringifyArray(const QString&, const QJSValue &) const;
    QueryItems stringifyValue(const QString&, const QJSValue &) const;
};

} } }

#endif // SERIALIZATION_H
