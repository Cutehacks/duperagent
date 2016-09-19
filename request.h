// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef REQUEST_H
#define REQUEST_H

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtQml/QJSValue>

#include "qpm.h"

class QHttpMultiPart;
class QQmlEngine;

namespace com { namespace cutehacks { namespace duperagent {

typedef QHash<QString, QByteArray> ContentTypeMap;
class Promise;

class RequestPrototype : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString method READ method WRITE setMethod)
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(QJSValue data READ data WRITE setData)
    Q_PROPERTY(QJSValue headers READ headers WRITE setHeaders)

public:
    enum Method {
        Head    = QNetworkAccessManager::HeadOperation,
        Post    = QNetworkAccessManager::PostOperation,
        Get     = QNetworkAccessManager::GetOperation,
        Put     = QNetworkAccessManager::PutOperation,
        Patch   = QNetworkAccessManager::CustomOperation,
        Delete  = QNetworkAccessManager::DeleteOperation,
    };

    enum ErrorType {
        Error,
        InternalError,
        RangeError,
        ReferenceError,
        SyntaxError,
        TypeError,
        URIError
    };

    RequestPrototype(QQmlEngine *, Method, const QUrl &);
    ~RequestPrototype();

    Q_INVOKABLE QJSValue use(QJSValue fn);
    Q_INVOKABLE QJSValue timeout(int ms);
    Q_INVOKABLE QJSValue clearTimeout();
    Q_INVOKABLE QJSValue abort();
    Q_INVOKABLE QJSValue set(const QJSValue&, const QJSValue& = QJSValue());
    Q_INVOKABLE QJSValue unset(const QString&);
    Q_INVOKABLE QJSValue type(const QJSValue&);
    Q_INVOKABLE QJSValue accept(const QJSValue&);
    Q_INVOKABLE QJSValue auth(const QString&, const QString&);
    Q_INVOKABLE QJSValue redirects(int);
    Q_INVOKABLE QJSValue query(const QJSValue&);
    Q_INVOKABLE QJSValue field(const QJSValue&, const QJSValue& = QJSValue());
    Q_INVOKABLE QJSValue attach(const QJSValue&, const QJSValue& = QJSValue(),
                                const QJSValue& = QJSValue());
    Q_INVOKABLE QJSValue send(const QJSValue&);
    Q_INVOKABLE QJSValue withCredentials();
    Q_INVOKABLE QJSValue on(const QJSValue&, const QJSValue&);
    Q_INVOKABLE QJSValue end(QJSValue callback);
    Q_INVOKABLE QJSValue then(QJSValue = QJSValue(), QJSValue = QJSValue());
    Q_INVOKABLE void endCallback(QJSValue, QJSValue);
    Q_INVOKABLE void executor(QJSValue, QJSValue);

    inline QJSValue self() const { return m_self; }

    QString method() const;
    void setMethod(const QString&);

    QString url() const;
    void setUrl(const QString&);

    QJSValue &data();
    void setData(const QJSValue&);

    QJSValue &headers();
    void setHeaders(const QJSValue&);

signals:
    void started();
    void progress(int loaded, int total);
    void response(QJSValue);
    void end();

protected slots:
    void handleFinished();
    void handleUploadProgress(qint64, qint64);
    void handleDownloadProgress(qint64, qint64);
#ifndef QT_NO_SSL
    void handleEncrypted();
    void handleSslErrors(const QList<QSslError> &);
#endif

protected:
    void dispatchRequest();
    void timerEvent(QTimerEvent *event);
    void callAndCheckError(QJSValue, const QJSValueList &);
    QByteArray serializeData();
    QJSValue createError(const QString&, ErrorType type = Error);
    QJSValue createProgressEvent(bool, qint64, qint64);
    void emitEvent(const QString&, const QJSValue&);

private:
    Method m_method;
    QJSValue m_self;
    QQmlEngine *m_engine;
    QNetworkAccessManager *m_network;
    QNetworkRequest *m_request;
    QNetworkReply *m_reply;
    QHttpMultiPart *m_multipart;
    int m_timeout;
    int m_timer;
    int m_redirects;
    int m_redirectCount;
    QUrlQuery m_query;
    QJSValue m_callback;
    QJSValue m_data;
    QJSValue m_headers;
    QByteArray m_rawData;
    QJSValue m_error;
    QHash<QString, QJSValueList> m_listeners;
    QScopedPointer<Promise> m_promise;
    QPair<QJSValue, QJSValue> m_executor;
};

} } }

#endif // REQUEST_H
