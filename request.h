// Copyright 2015 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef REQUEST_H
#define REQUEST_H

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtQml/QJSValue>

#include "qpm.h"

class QHttpMultiPart;
class QQmlEngine;

QPM_BEGIN_NAMESPACE(com, cutehacks, duperagent)

typedef QHash<QString, QByteArray> ContentTypeMap;

class RequestPrototype : public QObject {
    Q_OBJECT

public:
    enum Method {
        Head    = QNetworkAccessManager::HeadOperation,
        Post    = QNetworkAccessManager::PostOperation,
        Get     = QNetworkAccessManager::GetOperation,
        Put     = QNetworkAccessManager::PutOperation,
        Patch   = QNetworkAccessManager::CustomOperation,
        Delete  = QNetworkAccessManager::DeleteOperation,
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
    Q_INVOKABLE QJSValue end(QJSValue callback);

    inline QJSValue self() const { return m_self; }

signals:
    void progress(int loaded, int total);
    void response(QJSValue);
    void end();

protected slots:
    void handleFinished();

protected:
    void dispatchRequest();
    void timerEvent(QTimerEvent *event);
    QByteArray serializeData();

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
    QByteArray m_rawData;
};

QPM_END_NAMESPACE(com, cutehacks, duperagent)

#endif // REQUEST_H
