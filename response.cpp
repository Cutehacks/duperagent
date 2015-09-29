// Copyright 2015 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QRegExp>
#include <QtCore/QTextCodec>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtQml/QQmlEngine>

#include "response.h"

QPM_BEGIN_NAMESPACE(com, cutehacks, duperagent)

ResponsePrototype::ResponsePrototype(QQmlEngine *engine, QNetworkReply *reply) : QObject(0),
    m_engine(engine),
    m_reply(reply)
{
    QString type = m_reply->header(QNetworkRequest::ContentTypeHeader).toString();

    QRegExp charsetRegexp(".*charset=(.*)[\\s]*", Qt::CaseInsensitive, QRegExp::RegExp2);
    if (charsetRegexp.exactMatch(type)) {
        m_charset = charsetRegexp.capturedTexts().at(1);
    }

    QByteArray data = m_reply->readAll();

    QTextCodec *text = QTextCodec::codecForName(m_charset.toLatin1());
    m_text = text ? text->makeDecoder()->toUnicode(data) : QString::fromUtf8(data);

    if (type.contains("application/json")) {
        // TODO: add error handling
        QJsonDocument doc = QJsonDocument::fromJson(data, 0);
        m_body = parseJsonDocument(doc);
    } else if (type.contains("application/x-www-form-urlencoded")) {
        // TODO: Implement parsing of form-urlencoded
    } else if (type.contains("multipart/form-data")) {
        // TODO: Implement parsing of form-data
    }

    m_header = m_engine->newObject();
    QList<QNetworkReply::RawHeaderPair>::const_iterator it = m_reply->rawHeaderPairs().cbegin();
    for(; it != m_reply->rawHeaderPairs().cend(); it++) {
        m_header.setProperty(
                    QString::fromUtf8((*it).first),
                    QString::fromUtf8((*it).second));
    }

    m_engine->setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
}

ResponsePrototype::~ResponsePrototype()
{
    delete m_reply;
}

int ResponsePrototype::statusType() const {
    return statusCode() / 100;
}

int ResponsePrototype::statusCode() const {
    QVariant var = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    bool ok;
    int i = var.toInt(&ok);
    return ok ? i : 0;
}

bool ResponsePrototype::typeEquals(int type) const
{
    return statusType() == type;
}

bool ResponsePrototype::statusEquals(int code) const
{
    QVariant var = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    bool ok;
    int i = var.toInt(&ok);
    return ok && i == code;
}

bool ResponsePrototype::info() const { return typeEquals(1); }
bool ResponsePrototype::ok() const { return typeEquals(2); }
bool ResponsePrototype::clientError() const { return typeEquals(4); }
bool ResponsePrototype::serverError() const { return typeEquals(5); }
bool ResponsePrototype::error() const { return typeEquals(4) || typeEquals(5); }

bool ResponsePrototype::accepted() const { return statusEquals(202); }
bool ResponsePrototype::noContent() const { return statusEquals(204); }
bool ResponsePrototype::badRequest() const { return statusEquals(400); }
bool ResponsePrototype::unauthorized() const { return statusEquals(401); }
bool ResponsePrototype::notAcceptable() const { return statusEquals(406); }
bool ResponsePrototype::notFound() const { return statusEquals(404); }
bool ResponsePrototype::forbidden() const { return statusEquals(403); }

QString ResponsePrototype::text() const
{
    return m_text;
}

QString ResponsePrototype::charset() const
{
    return m_charset;
}

QJSValue ResponsePrototype::body() const
{
    return m_body;
}


QJSValue ResponsePrototype::header() const
{
    return m_header;
}

QJSValue ResponsePrototype::parseJsonDocument(const QJsonDocument &doc) const
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

QJSValue ResponsePrototype::parseJsonArray(const QJsonArray &array) const {
    QJSValue a = m_engine->newArray(array.size());
    int i = 0;
    for (QJsonArray::ConstIterator it = array.constBegin(); it != array.constEnd(); it++) {
        a.setProperty(i++, parseJsonValue(*it));
    }
    return a;
}

QJSValue ResponsePrototype::parseJsonObject(const QJsonObject &object) const
{
    QJSValue o = m_engine->newObject();
    for (QJsonObject::ConstIterator it = object.constBegin(); it != object.constEnd(); it++) {
        o.setProperty(it.key(), parseJsonValue(it.value()));
    }
    return o;
}

QJSValue ResponsePrototype::parseJsonValue(const QJsonValue &val) const
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

QPM_END_NAMESPACE(com, cutehacks, duperagent)
