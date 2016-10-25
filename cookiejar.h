// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <QtNetwork/QNetworkCookieJar>

namespace com { namespace cutehacks { namespace duperagent {

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT

public:
    CookieJar(const QString &path, QObject *parent = 0);
    ~CookieJar();

    bool insertCookie(const QNetworkCookie &);
    bool deleteCookie(const QNetworkCookie &);

    QString cookies() const;
    void addCookie(const QString &);

    void setPersistSessions(bool);
    bool persistSessions() const { return m_persistSessions; };

    void clearAll();

protected:
    void save() const;
    void load();

private:
    QString m_savePath;
    bool m_persistSessions;
};

} } }

#endif // COOKIEJAR_H
