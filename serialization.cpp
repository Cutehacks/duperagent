// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QStringBuilder>
#include <QtCore/QUrlQuery>
#include <QtQml/QQmlEngine>

#include "serialization.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
#include "jsvalueiterator.h"
#else
#include <QtQml/QJSValueIterator>
typedef QJSValueIterator JSValueIterator;
#endif

namespace com { namespace cutehacks { namespace duperagent {

static const QChar EQUALS('=');
static const QChar OPEN_SQUARE('[');
static const QChar CLOSE_SQUARE(']');

BodyCodec::BodyCodec(QQmlEngine *engine) : m_engine(engine) {}

JsonCodec::JsonCodec(QQmlEngine *engine) : BodyCodec(engine) {}

QJSValue JsonCodec::parse(const QByteArray &data) {
    QJsonDocument doc = QJsonDocument::fromJson(data, 0);
    return parseJsonDocument(doc);
}

QJSValue JsonCodec::parseJsonDocument(const QJsonDocument &doc)
{
    if (doc.isObject()) {
        return parseJsonObject(doc.object());
    } else if (doc.isArray()) {
        return parseJsonArray(doc.array());
    } else if(doc.isNull()){
        return QJSValue(QJSValue::NullValue);
    } else {
        return QJSValue();
    }
}

QJSValue JsonCodec::parseJsonArray(const QJsonArray &array) {
    QJSValue a = m_engine->newArray(array.size());
    int i = 0;
    for (QJsonArray::ConstIterator it = array.constBegin(); it != array.constEnd(); it++) {
        a.setProperty(i++, parseJsonValue(*it));
    }
    return a;
}

QJSValue JsonCodec::parseJsonObject(const QJsonObject &object)
{
    QJSValue o = m_engine->newObject();
    for (QJsonObject::ConstIterator it = object.constBegin(); it != object.constEnd(); it++) {
        o.setProperty(it.key(), parseJsonValue(it.value()));
    }
    return o;
}

QJSValue JsonCodec::parseJsonValue(const QJsonValue &val)
{
    switch (val.type()) {
    case QJsonValue::Array:
        return parseJsonArray(val.toArray());
    case QJsonValue::Object:
        return parseJsonObject(val.toObject());
    case QJsonValue::Bool:
        return QJSValue(val.toBool());
    case QJsonValue::Double:
        return QJSValue(val.toDouble());
    case QJsonValue::String:
        return QJSValue(val.toString());
    case QJsonValue::Undefined:
        return QJSValue(QJSValue::UndefinedValue);
    case QJsonValue::Null:
        return QJSValue(QJSValue::NullValue);
    }
}

QByteArray JsonCodec::stringify(const QJSValue &json)
{
    QJsonDocument doc;

    if (json.isObject()) {
        doc.setObject(stringifyObject(json));
    } else if (json.isArray()) {
        doc.setArray(stringifyArray(json));
    }

    return doc.toJson(QJsonDocument::Compact);
}

QJsonObject JsonCodec::stringifyObject(const QJSValue &json) const
{
    QJsonObject object;
    JSValueIterator it(json);
    while (it.next()) {
        object.insert(it.name(), stringifyValue(it.value()));
    }
    return object;
}

QJsonArray JsonCodec::stringifyArray(const QJSValue &json) const
{
    QJsonArray array;
    JSValueIterator it(json);
    while (it.next()) {
        if (it.hasNext()) // skip last item which is length
            array.append(stringifyValue(it.value()));
    }
    return array;
}

QJsonValue JsonCodec::stringifyValue(const QJSValue &json) const
{
    if (json.isArray()) {
        return QJsonValue(stringifyArray(json));
    } else if (json.isObject()) {
        QJSValue toJSON = json.property("toJSON");
        if (toJSON.isCallable())
            return stringifyValue(toJSON.callWithInstance(json));
        else
            return QJsonValue(stringifyObject(json));
    } else if (json.isBool()) {
        return QJsonValue(json.toBool());
    } else if (json.isNumber()) {
        return QJsonValue(json.toNumber());
    } else {
        return QJsonValue(json.toString());
    }
}

FormUrlEncodedCodec::FormUrlEncodedCodec(QQmlEngine *engine) : BodyCodec(engine)
{ }

QByteArray FormUrlEncodedCodec::stringify(const QJSValue &json)
{
    QueryItems items;
    JSValueIterator it(json);
    while (it.next()) {
        QString key = it.name();
        items << stringifyValue(key, it.value());
    }

    QUrlQuery query;
    query.setQueryItems(items);
    return query.toString(QUrl::FullyEncoded).toUtf8();
}

QueryItems FormUrlEncodedCodec::stringifyObject(const QString& prefix, const QJSValue &json) const
{
    QueryItems items;
    JSValueIterator it(json);
    while (it.next()) {
        items << stringifyValue(prefix % OPEN_SQUARE % it.name() % CLOSE_SQUARE, it.value());
    }
    return items;
}

QueryItems FormUrlEncodedCodec::stringifyArray(const QString& prefix, const QJSValue &json) const
{
    QueryItems items;
    JSValueIterator it(json);
    while (it.next()) {
        if (it.hasNext()) //skip last item which is length
            items << stringifyValue(prefix % OPEN_SQUARE % it.name() % CLOSE_SQUARE, it.value());
    }
    return items;
}

QueryItems FormUrlEncodedCodec::stringifyValue(const QString& prefix, const QJSValue &json) const
{
    if (json.isArray()) {
        return stringifyArray(prefix, json);
    } else if (json.isObject()) {
        QJSValue toJSON = json.property("toJSON");
        if (toJSON.isCallable())
            return stringifyValue(prefix, toJSON.callWithInstance(json));
        else
            return stringifyObject(prefix, json);
    } else if (json.isNull()) {
        return QueryItems() << QPair<QString, QString>(prefix, QString());
    } else if (json.hasProperty("toJSON") && json.property("toJSON").isCallable()){
        return QueryItems() << QPair<QString, QString>(
                    prefix, json.property("toJSON").callWithInstance(json).toString());
    } else {
        return QueryItems() << QPair<QString, QString>(prefix, json.toString());
    }
}

QJSValue FormUrlEncodedCodec::parse(const QByteArray &str)
{
    QJSValue json = m_engine->newObject();
    return json;
}
} } }

