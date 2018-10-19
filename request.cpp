// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QBuffer>
#include <QtCore/QMimeDatabase>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpMultiPart>
#include <QtQml/QQmlEngine>

#ifndef QT_NO_SSL
#include <QtNetwork/QSslError>
#include "ssl.h"
#endif

#include "request.h"
#include "response.h"
#include "config.h"
#include "serialization.h"
#include "promise.h"
#include "networkactivityindicator.h"
#include "multipartsource.h"
#include "duperagent.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
#include "jsvalueiterator.h"
#else
#include <QtQml/QJSValueIterator>
typedef QJSValueIterator JSValueIterator;
#endif

namespace com { namespace cutehacks { namespace duperagent {

ContentTypeMap contentTypes;

static const QString EVENT_REQUEST =    QStringLiteral("request");
static const QString EVENT_PROGRESS =   QStringLiteral("progress");
static const QString EVENT_END =        QStringLiteral("end");
static const QString EVENT_RESPONSE =   QStringLiteral("response");
static const QString EVENT_SECURE =     QStringLiteral("secureconnect");

static const QString METHOD_HEAD =      QStringLiteral("HEAD");
static const QString METHOD_POST =      QStringLiteral("POST");
static const QString METHOD_GET =       QStringLiteral("GET");
static const QString METHOD_PUT =       QStringLiteral("PUT");
static const QString METHOD_PATCH =     QStringLiteral("PATCH");
static const QString METHOD_DELETE =    QStringLiteral("DELETE");

static inline uint percent(qint64 loaded, qint64 total) {
    if (total > 0)
        return int(loaded / (double)total * 100);
    return 0;
}

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
    m_redirectCount(0),
    m_promise(0),
    m_responseType(duperagent::ResponseType::Auto)
{
    Config::instance()->init(m_engine);
    m_request = new QNetworkRequest(QUrl(url.toString()));
    m_query = QUrlQuery(url);
    m_engine->setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
    m_self = m_engine->newQObject(this);
    m_headers = m_engine->newObject();
}

RequestPrototype::~RequestPrototype()
{
    delete m_request;
    delete m_multipart;
}

QJSValue RequestPrototype::use(QJSValue fn)
{
    if (fn.isCallable()) {
        callAndCheckError(fn, QJSValueList() << self());
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
            m_headers.setProperty(
                        it.name(),
                        it.value().toString());
        }
    } else {
        m_headers.setProperty(
                    field.toString(),
                    val.toString());
    }

    return self();
}

QJSValue RequestPrototype::unset(const QString &field)
{
    m_headers.deleteProperty(field.toUtf8());
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

QJSValue RequestPrototype::cacheSave(bool enabled)
{
    m_request->setAttribute(QNetworkRequest::CacheSaveControlAttribute,
                            enabled);
    return self();
}

QJSValue RequestPrototype::cacheLoad(int value)
{
    m_request->setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                            value);
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

    QString fname = filename.isString() ? filename.toString() : QString();
    QString contentType;
    QIODevice *bodyDevice(0);

    if (path.isQObject()) {
        AbstractMultipartSource *source =
            qobject_cast<AbstractMultipartSource *>(path.toQObject());

        if (!source) {
            qWarning("Object must subclass AbstractMultipartSource");
            return self();
        }

        QBuffer *buffer = new QBuffer;
        buffer->setData(source->data());
        buffer->open(QIODevice::ReadOnly);
        bodyDevice = buffer;

        contentType = source->mimeType();
    } else {
        QUrl url(path.toString());
        QFile *file = new QFile(url.isLocalFile() ? url.toLocalFile() : path.toString(),
                                m_multipart);

        if (!file->exists()) {
            qWarning("File does not exist");
            return self();
        }

        if (!file->open(QIODevice::ReadOnly)) {
            qWarning("Could not open file for reading");
            return self();
        }

        bodyDevice = file;

        QFileInfo fileInfo(*file);
        if (fname.isEmpty())
            fname = fileInfo.fileName();

        // TODO: Move the mimeDB somewhere more static
        QMimeDatabase mimeDB;
        contentType = mimeDB.mimeTypeForFile(fileInfo).name();
    }

    dispositionHeader += " filename=\"" + fname + "\"";

    QHttpPart attachment;
    attachment.setHeader(QNetworkRequest::ContentDispositionHeader, dispositionHeader);

    if (!contentType.isEmpty())
        attachment.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(contentType));

    m_attachments.add(bodyDevice);
    attachment.setBodyDevice(bodyDevice);

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

QJSValue RequestPrototype::on(const QJSValue &event, const QJSValue &handler)
{
    if (!event.isString()) {
        qWarning("'on' expects a string as first argument");
        return self();
    }
    if (!handler.isCallable()) {
        qWarning("'on' expects a function as second argument");
        return self();
    }

    QString name = event.toString().toLower();
    m_listeners[name].append(handler);

    return self();
}

QJSValue RequestPrototype::end(QJSValue callback)
{
    m_callback = callback;

    dispatchRequest();

    return self();
}

void RequestPrototype::endCallback(QJSValue err, QJSValue res)
{
    if (err.isError()) {
        m_executor.second.call(QJSValueList() << err);
    } else {
        m_executor.first.call(QJSValueList() << res);
    }
}

void RequestPrototype::executor(QJSValue resolve, QJSValue reject)
{
    m_executor.first = resolve;
    m_executor.second = reject;
    end(self().property("endCallback"));
}

QJSValue RequestPrototype::then(QJSValue onFulfilled, QJSValue onRejected)
{
    if (!m_promise) {
        m_promise.reset(new Promise(m_engine, self().property("executor")));
        m_engine->setObjectOwnership(m_promise.data(), QQmlEngine::CppOwnership);
    }

    return m_promise->then(onFulfilled, onRejected);
}

QString RequestPrototype::method() const
{
    switch (m_method) {
    case Head:
        return METHOD_HEAD;
    case Post:
        return METHOD_POST;
    case Get:
        return METHOD_GET;
    case Put:
        return METHOD_PUT;
    case Patch:
        return METHOD_PATCH;
    case Delete:
        return METHOD_DELETE;
    }
    return QString();
}

void RequestPrototype::setMethod(const QString &method)
{
    QString upper = method.toUpper().trimmed();
    if (upper == METHOD_GET) {
        m_method = Get;
    } else if (upper == METHOD_POST) {
        m_method = Post;
    } else if (upper == METHOD_PUT) {
        m_method = Put;
    } else if (upper == METHOD_PATCH) {
        m_method = Patch;
    } else if (upper == METHOD_DELETE) {
        m_method = Delete;
    } else if (upper == METHOD_HEAD) {
        m_method = Head;
    } else {
        qWarning("Invalid method: %s", qUtf8Printable(method));
    }
}

QString RequestPrototype::url() const
{
    return m_request->url().toString();
}

void RequestPrototype::setUrl(const QString &u)
{
    QUrl url(u, QUrl::TolerantMode);
    if (url.isValid()) {
        m_request->setUrl(url);
    } else {
        qWarning("Invalid url: %s", qUtf8Printable(url.errorString()));
    }
}

QJSValue &RequestPrototype::data()
{
    return m_data;
}

void RequestPrototype::setData(const QJSValue &data)
{
   m_data = data;
}

QJSValue &RequestPrototype::headers()
{
    return m_headers;
}

void RequestPrototype::setHeaders(const QJSValue &headers)
{
    m_headers = headers;
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

    JSValueIterator it(m_headers);
    while (it.next()) {
        m_request->setRawHeader(
                    it.name().toUtf8(),
                    it.value().toString().toUtf8());
    }


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

    NetworkActivityIndicator::instance()->incrementActivityCount();

    emit started();
    emitEvent(EVENT_REQUEST, self());

    connect(m_reply, SIGNAL(finished()), this, SLOT(handleFinished()));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(handleDownloadProgress(qint64, qint64)));
    connect(m_reply, SIGNAL(uploadProgress(qint64,qint64)),
            this, SLOT(handleUploadProgress(qint64, qint64)));
#ifndef QT_NO_SSL
    connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(handleSslErrors(QList<QSslError>)));
    connect(m_reply, SIGNAL(encrypted()),
            this, SLOT(handleEncrypted()));
#endif

    if (m_timeout > 0)
        m_timer = startTimer(m_timeout);
}

void RequestPrototype::handleFinished()
{
    killTimer(m_timer);
    NetworkActivityIndicator::instance()->decrementActivityCount();

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

    emitEvent(EVENT_END, QJSValue::UndefinedValue);

    // clean up any attachment bodies
    m_attachments.clear();

    QJSValueList args;

    ResponsePrototype *rep = new ResponsePrototype(m_engine, m_reply, m_responseType);

    if (m_error.isError()) {
        m_error.setProperty("response", m_engine->newQObject(rep));
    }

    QJSValue res = m_engine->newQObject(rep);
    args << m_error << m_engine->newQObject(rep);

    emitEvent(EVENT_RESPONSE, res);

    if (m_callback.isCallable()) {
        callAndCheckError(m_callback, args);
    } else {
        qWarning("%s is not callable", qUtf8Printable(m_callback.toString()));
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

QJSValue RequestPrototype::createProgressEvent(bool upload, qint64 loaded, qint64 total)
{
    QJSValue event = m_engine->newObject();
    event.setProperty("direction", upload ? "upload" : "download");
    event.setProperty("loaded",  (uint)loaded);
    event.setProperty("total",  (uint)total);
    event.setProperty("percent",  percent(loaded, total));
    return event;
}


void RequestPrototype::handleUploadProgress(qint64 sent, qint64 total)
{
    emitEvent(EVENT_PROGRESS, createProgressEvent(true, sent, total));
    emit progress(sent, total);
}

void RequestPrototype::handleDownloadProgress(qint64 received, qint64 total)
{
    emitEvent(EVENT_PROGRESS, createProgressEvent(false, received, total));
    emit progress(received, total);
}

void RequestPrototype::emitEvent(const QString &name, const QJSValue &event)
{
    QJSValueList args;
    args.append(event);

    QJSValueList listeners = m_listeners[name];
    for (QJSValueList::iterator it = listeners.begin(); it != listeners.end(); it++)
        callAndCheckError(*it, args);
}

QJSValue RequestPrototype::responseType(int responseType)
{
    m_responseType = responseType;
    return self();
}


#ifndef QT_NO_SSL
void RequestPrototype::handleEncrypted()
{
    // return if no one is listening
    if (m_listeners[EVENT_SECURE].length() == 0)
        return;

    SecureConnectEvent *event = new SecureConnectEvent(m_engine, m_reply->sslConfiguration());
    emitEvent(EVENT_SECURE, event->self());
}

void RequestPrototype::handleSslErrors(const QList<QSslError> &)
{

}
#endif

void RequestPrototype::timerEvent(QTimerEvent *)
{
    m_error = createError(QString("Timeout of %1 ms exceeded").arg(m_timeout));
    abort();
}

void RequestPrototype::callAndCheckError(QJSValue fn, const QJSValueList &args)
{
    QJSValue result = fn.call(args);
    if (result.isError()) {
        QString fileName = result.property("fileName").toString();
        QString lineNumber = result.property("lineNumber").toString();
        qWarning("%s: %s (%s)",
            qUtf8Printable(result.toString()),
            qUtf8Printable(fileName),
            qUtf8Printable(lineNumber));
    }
}

} } }
