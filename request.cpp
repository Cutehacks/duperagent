// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.


#include <QtDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QBuffer>
#include <QtCore/QMimeDatabase>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpMultiPart>
#ifndef QT_NO_SSL
#include <QtNetwork/QSslError>
#endif
#include <QtQml/QQmlEngine>

#include "request.h"
#include "response.h"
#include "config.h"
#include "serialization.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
#include "jsvalueiterator.h"
#else
#include <QtQml/QJSValueIterator>
typedef QJSValueIterator JSValueIterator;
#endif

namespace com { namespace cutehacks { namespace duperagent {

ContentTypeMap contentTypes;

RequestPrototype::RequestPrototype(QQmlEngine *engine, Method method, const QUrl &url) :
    QObject(0),
    m_method(method),
    m_engine(engine),
    m_network(engine->networkAccessManager()),
    m_request(0),
    m_reply(0),
    m_multipart(0),
    m_timeout(-1),
    m_timer(0),
    m_redirects(5),
    m_redirectCount(0)
{
    Config::instance()->init(m_engine);
    m_request = new QNetworkRequest(QUrl(url.toString()));
    m_query = QUrlQuery(url);
    m_engine->setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
    m_self = m_engine->newQObject(this);
}

RequestPrototype::~RequestPrototype()
{
    delete m_request;
    delete m_multipart;
}

QJSValue RequestPrototype::use(QJSValue fn)
{
    if (fn.isCallable()) {
        fn.call(QJSValueList() << self());
    } else {
        qWarning("'use' expects a function");
    }
    return self();
}

QJSValue RequestPrototype::timeout(int ms)
{
    m_timeout = ms;
    return self();
}

QJSValue RequestPrototype::clearTimeout()
{
    m_timeout = -1;
    return self();
}

QJSValue RequestPrototype::abort()
{
    if (m_reply && m_reply->isRunning())
        m_reply->abort();
    return self();
}

QJSValue RequestPrototype::set(const QJSValue &field, const QJSValue &val)
{
    if (field.isObject()) {
        JSValueIterator it(field);
        while (it.next()) {
            m_request->setRawHeader(
                        it.name().toUtf8(),
                        it.value().toString().toUtf8());
        }
    } else {
        m_request->setRawHeader(
                    field.toString().toUtf8(),
                    val.toString().toUtf8());
    }

    return self();
}

QJSValue RequestPrototype::unset(const QString &field)
{
    m_request->setRawHeader(
                field.toUtf8(),
                QByteArray());

    return self();
}

QJSValue RequestPrototype::type(const QJSValue &type)
{
    QString t = type.toString();
    if (contentTypes.contains(t)) {
        m_request->setHeader(QNetworkRequest::ContentTypeHeader,
                             contentTypes.value(t));
    } else {
        m_request->setHeader(QNetworkRequest::ContentTypeHeader, t);
    }
    return self();
}

QJSValue RequestPrototype::accept(const QJSValue &type)
{
    const QByteArray ACCEPT_HEADER("Accept");
    QByteArray t = type.toString().toUtf8();
    if (contentTypes.contains(t)) {
        m_request->setRawHeader(ACCEPT_HEADER, contentTypes.value(t));
    } else {
        m_request->setRawHeader(ACCEPT_HEADER, t);
    }
    return self();
}

QJSValue RequestPrototype::auth(const QString &user, const QString &password)
{
    const QByteArray AUTH_HEADER("Authorization");
    QByteArray value = QByteArray("Basic ").append(
                (user + ":" + password)
                .toUtf8()
                .toBase64(QByteArray::Base64UrlEncoding));

    m_request->setRawHeader(AUTH_HEADER, value);
    return self();
}

QJSValue RequestPrototype::redirects(int redirects)
{
    m_redirects = redirects;
    return self();
}


QJSValue RequestPrototype::query(const QJSValue &query)
{
    if (query.isObject()) {
        JSValueIterator it(query);
        while (it.next()) {
            m_query.addQueryItem(
                        it.name(),
                        it.value().toString());
        }
    } else if (query.isString()) {
        QUrlQuery parsed(query.toString());
        QList<QPair<QString, QString> > items =
                parsed.queryItems(QUrl::FullyDecoded);
        QPair<QString, QString> pair;
        foreach(pair, items) {
            m_query.addQueryItem(pair.first, pair.second);
        }
    } else {
        qWarning("'query' expects an object or string");
    }

    return self();
}

QJSValue RequestPrototype::field(const QJSValue &name, const QJSValue &value)
{
    if (!m_multipart)
        m_multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);

    QString dispositionHeader("form-data; name=\"");
    dispositionHeader += name.toString();
    dispositionHeader += "\";";

    QHttpPart field;
    field.setHeader(QNetworkRequest::ContentDispositionHeader, dispositionHeader);
    field.setBody(value.toString().toUtf8());

    m_multipart->append(field);

    return self();
}

QJSValue RequestPrototype::attach(const QJSValue &name, const QJSValue &path,
                                  const QJSValue &filename)
{
    if (!m_multipart)
        m_multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);

    QString dispositionHeader("form-data; name=\"");
    dispositionHeader += name.toString();
    dispositionHeader += "\";";

    QUrl url(path.toString());
    QFile *file = new QFile(url.toLocalFile(), m_multipart);

    if (!file->exists()) {
        qWarning("File does not exist");
        return self();
    }

    if (!file->open(QIODevice::ReadOnly)) {
        qWarning("Could not open file for reading");
        return self();
    }

    QFileInfo fileInfo(*file);

    QString fname = filename.isString() ? filename.toString() : fileInfo.fileName();
    dispositionHeader += " filename=\"" + fname + "\"";

    QHttpPart attachment;
    attachment.setHeader(QNetworkRequest::ContentDispositionHeader, dispositionHeader);

    // TODO: Move the mimeDB somewhere more static
    QMimeDatabase mimeDB;
    QString contentType = mimeDB.mimeTypeForFile(fileInfo).name();
    if (!contentType.isEmpty())
        attachment.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(contentType));

    attachment.setBodyDevice(file);

    m_multipart->append(attachment);

    return self();
}

QJSValue RequestPrototype::send(const QJSValue &data)
{
    QByteArray type = m_request->header(QNetworkRequest::ContentTypeHeader)
            .toByteArray();

    if (data.isObject() && m_data.isObject()) {
        // merge with existing data
        JSValueIterator it(data);
        while (it.next()) {
            m_data.setProperty(it.name(), it.value());
        }
    } else if(data.isString()) {
        if (type.isEmpty()) {
            type = contentTypes["form"];
        }
        if (type == contentTypes["form"]) {
            if (m_data.isString()) {
                m_data = QJSValue(m_data.toString() + "&" + data.toString());
            } else {
                m_data = data;
            }
        } else {
            if (m_data.isString()) {
                m_data = QJSValue(m_data.toString() + data.toString());
            } else {
                m_data = data;
            }
        }
    } else {
        m_data = data;
    }

    if (type.isEmpty())
        type = contentTypes["json"];

    m_request->setHeader(QNetworkRequest::ContentTypeHeader, type);

    return self();
}

QJSValue RequestPrototype::withCredentials()
{
    return self();
}

QJSValue RequestPrototype::end(QJSValue callback)
{
    m_callback = callback;

    dispatchRequest();

    return self();
}

QByteArray RequestPrototype::serializeData()
{
    QByteArray type = m_request->header(
                QNetworkRequest::ContentTypeHeader).toByteArray();

    if (type.contains(contentTypes["json"])) {
        JsonCodec json(m_engine);
        return json.stringify(m_data);
    } else if (type.contains(contentTypes["form"])) {
        FormUrlEncodedCodec urlencoded(m_engine);
        return urlencoded.stringify(m_data);
    }
    return m_data.toString().toUtf8();
}

void RequestPrototype::dispatchRequest()
{
    QUrl url = m_request->url();
    url.setQuery(m_query);
    m_request->setUrl(url);

    switch (m_method) {
    case Get:
        m_reply = m_network->get(*m_request);
        break;
    case Post:
        if (m_multipart) {
            m_reply = m_network->post(*m_request, m_multipart);
        } else {
            m_reply = m_network->post(*m_request, serializeData());
        }
        break;
    case Put:
        if (m_multipart) {
            m_reply = m_network->put(*m_request, m_multipart);
        } else {
            m_reply = m_network->put(*m_request, serializeData());
        }
        break;
    case Delete:
        m_reply = m_network->deleteResource(*m_request);
        break;
    case Patch:
        m_request->setAttribute(QNetworkRequest::CustomVerbAttribute, "PATCH");
        m_rawData = serializeData();
        m_reply = m_network->sendCustomRequest(*m_request, "PATCH",
                                               new QBuffer(&m_rawData, this));
        break;
    case Head:
        m_reply = m_network->head(*m_request);
        break;
    default:
        qWarning("Unsupported method");
    }

    emit started();

    connect(m_reply, SIGNAL(finished()), this, SLOT(handleFinished()));
#ifndef QT_NO_SSL
    connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(handleSslErrors(QList<QSslError>)));
#endif

    if (m_timeout > 0)
        m_timer = startTimer(m_timeout);
}

void RequestPrototype::handleFinished()
{
    killTimer(m_timer);

    int status = m_reply->attribute(
                QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QVariant redir = m_reply->attribute(
                QNetworkRequest::RedirectionTargetAttribute);

    if (redir.isValid()) {
        m_redirectCount++;
        if (m_redirectCount <= m_redirects) {
            QUrl location = m_request->url().resolved(redir.toUrl());
            if (location.scheme() == "file") {
                qWarning("Invalid redirect URL scheme");
                return;
            }

            QNetworkRequest *req = new QNetworkRequest(*m_request);
            req->setUrl(location);
            delete m_request;
            m_request = req;
            m_reply->deleteLater();
            m_reply = 0;

            if (status >= 301 && status <= 303) {
                if (m_method == Post) {
                    // TODO: Strip Content-* headers

                    // TODO: Strip send data

                    m_multipart = 0;
                }

                if (status == 303 || m_method != Head) {
                    m_method = Get;
                }
            } else if (status != 307 && status != 308) {
                // Don't change methods on 307/308
                qWarning("Unhandled redirect status code");
                return;
            }
            dispatchRequest();
            return;
        } else {
            m_error = createError("Too many redirects");
        }
    }

    if (!m_error.isError() && m_reply->error() != QNetworkReply::NoError) {
        m_error = createError(m_reply->errorString());
        m_error.setProperty("code", m_reply->error());
        if (status >= 400) {
            m_error.setProperty("status", status);
            // TODO: Add "method" property
        }
    }

    QJSValueList args;

    ResponsePrototype *rep = new ResponsePrototype(m_engine, m_reply);
    args << m_error << m_engine->newQObject(rep);

    if (m_callback.isCallable()) {
        m_callback.call(args);
    } else {
        qWarning() << QString("%1 is not callable").arg(m_callback.toString());
    }
}

QJSValue RequestPrototype::createError(const QString &message, ErrorType type)
{
    QString err;
    switch (type) {
    case RangeError:
        err = "RangeError";
        break;
    case ReferenceError:
        err = "ReferenceError";
        break;
    case SyntaxError:
        err = "SyntaxError";
        break;
    case TypeError:
        err = "TypeError";
        break;
    case URIError:
        err = "URIError";
        break;
    case InternalError:
        err = "InternalError";
        break;
    case Error:
    default:
        err = "Error";
    }

    QString script = "new %1('%2');";
    return m_engine->evaluate(script.arg(err).arg(message));
}

#ifndef QT_NO_SSL
void RequestPrototype::handleSslErrors(const QList<QSslError> &)
{

}
#endif

void RequestPrototype::timerEvent(QTimerEvent *)
{
    m_error = createError(QString("Timeout of %1 ms exceeded").arg(m_timeout));
    abort();
}

} } }
