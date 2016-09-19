// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtNetwork/QSslCipher>
#include <QtQml/QQmlEngine>
#include <QtDebug>

#include "ssl.h"

namespace com { namespace cutehacks { namespace duperagent {

static QString formatAsHex(const QByteArray &data)
{
    QByteArray::const_iterator it = data.cbegin();
    QStringList parts;
    while (it != data.cend()) {
        parts.append(QString("%1").arg((uchar)*it, 2, 16, QLatin1Char('0')));
         it++;
    }
    return parts.join(":");

}

SslCertificate::SslCertificate(QQmlEngine *engine, const QSslCertificate& cert) :
    QObject(0),
    m_engine(engine),
    m_cert(cert)
{
    m_self = m_engine->newQObject(this);
    m_engine->setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
}

QJSValue SslCertificate::subject() const
{
    QJSValue subject = m_engine->newObject();
    subject.setProperty("C",    m_cert.subjectInfo(QSslCertificate::CountryName).join(" "));
    subject.setProperty("ST",   m_cert.subjectInfo(QSslCertificate::StateOrProvinceName).join(" "));
    subject.setProperty("L",    m_cert.subjectInfo(QSslCertificate::LocalityName).join(" "));
    subject.setProperty("O",    m_cert.subjectInfo(QSslCertificate::Organization).join(" "));
    subject.setProperty("OU",   m_cert.subjectInfo(QSslCertificate::OrganizationalUnitName).join(" "));
    subject.setProperty("CN",   m_cert.subjectInfo(QSslCertificate::CommonName).join(" "));
    return subject;
}

QJSValue SslCertificate::issuerInfo() const
{
    QJSValue issuerInfo = m_engine->newObject();
    issuerInfo.setProperty("C",     m_cert.issuerInfo(QSslCertificate::CountryName).join(" "));
    issuerInfo.setProperty("ST",    m_cert.issuerInfo(QSslCertificate::StateOrProvinceName).join(" "));
    issuerInfo.setProperty("L",     m_cert.issuerInfo(QSslCertificate::LocalityName).join(" "));
    issuerInfo.setProperty("O",     m_cert.issuerInfo(QSslCertificate::Organization).join(" "));
    issuerInfo.setProperty("OU",    m_cert.issuerInfo(QSslCertificate::OrganizationalUnitName).join(" "));
    issuerInfo.setProperty("CN",    m_cert.issuerInfo(QSslCertificate::CommonName).join(" "));
    return issuerInfo;
}

QJSValue SslCertificate::issuer() const
{
    // TODO: Implement chaining
    return QJSValue();
}

QByteArray SslCertificate::raw() const
{
    // TODO: See if we can expose this as a TypedArray somehow
    return m_cert.toDer();
}

QDateTime SslCertificate::valid_from() const
{
    return m_cert.effectiveDate();
}

QDateTime SslCertificate::valid_to() const
{
    return m_cert.expiryDate();
}

QString SslCertificate::fingerprint() const
{
    return formatAsHex(m_cert.digest(QCryptographicHash::Sha1));
}

QString SslCertificate::serialNumber() const
{
    return formatAsHex(QByteArray::fromHex(m_cert.serialNumber()));
}

SecureConnectEvent::SecureConnectEvent(QQmlEngine *engine, const QSslConfiguration &ssl) :
    QObject(0),
    m_engine(engine),
    m_ssl(ssl)
{
    m_self = m_engine->newQObject(this);
    m_engine->setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
}

QJSValue SecureConnectEvent::getPeerCertificate()
{
    SslCertificate *cert = new SslCertificate(m_engine, m_ssl.peerCertificate());
    return cert->self();
}

QString SecureConnectEvent::getProtocol()
{
    switch (m_ssl.sessionProtocol()) {
    case QSsl::SslV3:
        return QStringLiteral("SSLv3");
    case QSsl::TlsV1_0:
        return QStringLiteral("TLSv1");
    case QSsl::TlsV1_1:
        return QStringLiteral("TLSv1.1");
    case QSsl::TlsV1_2:
        return QStringLiteral("TLSv1.2");
    default:
        return QStringLiteral("unknown");
    }
}

QJSValue SecureConnectEvent::getCipher()
{
    QJSValue cipher = m_engine->newObject();
    cipher.setProperty("name", m_ssl.sessionCipher().name());
    cipher.setProperty("version", m_ssl.sessionCipher().protocolString());
    return cipher;
}

} } }
