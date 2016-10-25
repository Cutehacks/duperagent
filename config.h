// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef CONFIG_H
#define CONFIG_H

#include <QGlobalStatic>
#include <QtQml/QJSValue>

#include "qpm.h"

class QQmlEngine;

namespace com { namespace cutehacks { namespace duperagent {

class Config
{
public:
    Config();
    void init(QQmlEngine *);
    void setOptions(const QJSValue&);

    static Config* instance();

private:
    bool m_doneInit;
    bool m_noCache;
    bool m_noCookieJar;
    bool m_systemProxy;
    QString m_cachePath;
    qint64 m_maxCacheSize;
    QString m_cookieJarPath;
    bool m_persistSessionCookies;
};

} } }

#endif // CONFIG_H
