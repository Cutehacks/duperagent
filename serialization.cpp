#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtQml/QQmlEngine>

#include "serialization.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
#include "jsvalueiterator.h"
#else
#include <QtQml/QJSValueIterator>
typedef QJSValueIterator JSValueIterator;
#endif

namespace com { namespace cutehacks { namespace duperagent {

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

} } }

