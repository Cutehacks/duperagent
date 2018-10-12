// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QRegExp>
#include <QtCore/QTextCodec>
#include <QtNetwork/QNetworkReply>
#include <QtQml/QQmlEngine>
#include <QJsonObject>

#include "response.h"
#include "serialization.h"
#include "duperagent.h"

namespace com { namespace cutehacks { namespace duperagent {

ResponsePrototype::ResponsePrototype(QQmlEngine *engine, QNetworkReply *reply, int responseType) : QObject(0),
    m_engine(engine),
    m_reply(reply)
{
    QString type = m_reply->header(QNetworkRequest::ContentTypeHeader).toString();

    QRegExp charsetRegexp(".*charset=(.*)[\\s]*", Qt::CaseInsensitive, QRegExp::RegExp2);
    if (charsetRegexp.exactMatch(type)) {
        m_charset = charsetRegexp.capturedTexts().at(1);
    }

    if (m_reply->isReadable()) {
        QByteArray data = m_reply->readAll();

        QTextCodec *text = QTextCodec::codecForName(m_charset.toLatin1());
        m_text = text ? text->makeDecoder()->toUnicode(data) : QString::fromUtf8(data);

        switch (responseType)
        {
            case ResponseType::Text:
            {
                m_body = m_text;
                break;
            }
            case ResponseType::Json:
            {
                if (!type.contains("application/json")) {
                    m_body = QJsonObject();
                    break;
                }
                // TODO: add error handling
                JsonCodec json(m_engine);
                m_body.setValue(json.parse(data).toVariant());
                break;
            }
            case ResponseType::Blob:
            {
                m_body = data;
                break;
            }
            case ResponseType::ArrayBuffer:
            {
                m_body = data;
                break;
            }
            default:
            {
                if (type.contains("application/json")) {
                    // TODO: add error handling
                    JsonCodec json(m_engine);
                    m_body.setValue(json.parse(data).toVariant());
        //        } else if (type.contains("application/x-www-form-urlencoded")) {
                    // TODO: Implement parsing of form-urlencoded
        //        } else if (type.contains("multipart/form-data")) {
                    // TODO: Implement parsing of form-data
                } else if (type.contains("image/")) {
                    m_body = QString("data:%1;base64,%2")
                            .arg(type)
                            .arg(QString::fromLatin1(data.toBase64()));
                } else {
                    m_body = m_text;
                }
                break;
            }
        }
    }

    m_header = m_engine->newObject();
    QList<QNetworkReply::RawHeaderPair>::const_iterator it = m_reply->rawHeaderPairs().cbegin();
    for(; it != m_reply->rawHeaderPairs().cend(); it++) {
        m_header.setProperty(
                    QString::fromUtf8((*it).first).toLower(),
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

bool ResponsePrototype::fromCache() const
{
    return m_reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();
}

QString ResponsePrototype::text() const
{
    return m_text;
}

QString ResponsePrototype::charset() const
{
    return m_charset;
}

QVariant ResponsePrototype::body() const
{
    return m_body;
}


QJSValue ResponsePrototype::header() const
{
    return m_header;
}

} } }
