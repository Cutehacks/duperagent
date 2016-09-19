// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef SSLCERTIFICATE_H
#define SSLCERTIFICATE_H

#include <QObject>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslCertificate>
#include <QtQml/QJSValue>

class QQmlEngine;

namespace com { namespace cutehacks { namespace duperagent {

class SslCertificate : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QJSValue subject READ subject)
    Q_PROPERTY(QJSValue issuerInfo READ issuerInfo)
    Q_PROPERTY(QJSValue issuer READ issuer)
    Q_PROPERTY(QByteArray raw READ raw)
    Q_PROPERTY(QDateTime valid_from READ valid_from)
    Q_PROPERTY(QDateTime valid_to READ valid_to)
    Q_PROPERTY(QString fingerprint READ fingerprint)
    Q_PROPERTY(QString serialNumber READ serialNumber)

public:
    explicit SslCertificate(QQmlEngine *m_engine, const QSslCertificate&);
    inline QJSValue self() const { return m_self; }

    QJSValue subject() const;
    QJSValue issuerInfo() const;
    QJSValue issuer() const;
    QByteArray raw() const;
    QDateTime valid_from() const;
    QDateTime valid_to() const;
    QString fingerprint() const;
    QString serialNumber() const;

private:
    QJSValue m_self;
    QQmlEngine *m_engine;
    QSslCertificate m_cert;
};

class SecureConnectEvent : public QObject {
    Q_OBJECT

public:
    explicit SecureConnectEvent(QQmlEngine *, const QSslConfiguration&);
    inline QJSValue self() const { return m_self; }

    Q_INVOKABLE QJSValue getPeerCertificate();
    Q_INVOKABLE QString getProtocol();
    Q_INVOKABLE QJSValue getCipher();

private:
    QJSValue m_self;
    QQmlEngine *m_engine;
    QSslConfiguration m_ssl;
};

} } }

#endif // SSLCERTIFICATE_H
