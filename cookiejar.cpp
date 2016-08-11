// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtNetwork/QNetworkCookie>

#include "cookiejar.h"

namespace com { namespace cutehacks { namespace duperagent {

CookieJar::CookieJar(const QString &path, QObject *parent) :
    QNetworkCookieJar(parent),
    m_savePath(path)
{
    load();
}

CookieJar::~CookieJar()
{
    save();
}

bool CookieJar::insertCookie(const QNetworkCookie &cookie)
{
    if (QNetworkCookieJar::insertCookie(cookie)) {
        save(); // too aggressive to save on each insert?
        return true;
    }
    return false;
}

bool CookieJar::deleteCookie(const QNetworkCookie &cookie)
{
    if (QNetworkCookieJar::deleteCookie(cookie)) {
        save();
        return true;
    }
    return false;
}

void CookieJar::save() const
{
    QFile file(m_savePath);
    QDir dir = QFileInfo(file).dir();

    if (!dir.mkpath(dir.absolutePath())) {
        qWarning("Could not create path for writing: %s", qUtf8Printable(dir.path()));
        return;
    }

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Could not open file for writing: %s", qUtf8Printable(m_savePath));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    QList<QNetworkCookie> cookies = allCookies();
    foreach (QNetworkCookie c, cookies) {
        if (!c.isSessionCookie())
            out << c.toRawForm() << endl;
    }

    file.close();
}

void CookieJar::load()
{
    QFile file(m_savePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    QList<QNetworkCookie> cookies;
    while (!in.atEnd()) {
        QByteArray line = in.readLine().toUtf8();
        cookies << QNetworkCookie::parseCookies(line);
    }
    file.close();

    setAllCookies(cookies);
}

void CookieJar::addCookie(const QString &cookieString)
{
    QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(cookieString.toUtf8());

    if (newCookies.length() == 0)
        return;

    QList<QNetworkCookie> cookies = allCookies();
    foreach (QNetworkCookie cookie, newCookies) {
        bool found = false;
        foreach (QNetworkCookie existing, cookies) {
            if (cookie.hasSameIdentifier(existing)) {
                found = true;
                if (!existing.isHttpOnly())
                    insertCookie(cookie);
                break;
            }
        }
        if (!found)
            insertCookie(cookie);
    }
}

void CookieJar::clearAll()
{
    setAllCookies(QList<QNetworkCookie>());
    save();
}

QString CookieJar::cookies() const
{
    QStringList cookieString;
    QList<QNetworkCookie> cookies = allCookies();
    foreach (QNetworkCookie c, cookies) {
        if (!c.isHttpOnly()) {
            cookieString << c.toRawForm();
        }
    }

    return cookieString.join(";");
}

} } }

