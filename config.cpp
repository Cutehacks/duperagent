// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QStandardPaths>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxyFactory>
#include <QtCore/QString>
#include <QtQml/QQmlEngine>

#include "config.h"
#include "cookiejar.h"

namespace com { namespace cutehacks { namespace duperagent {

static const char *PROP_CACHE           = "cache";
static const char *PROP_CACHE_MAX_SIZE  = "maxSize";
static const char *PROP_CACHE_LOC       = "location";

static const char *PROP_COOKIE_JAR      = "cookieJar";
static const char *PROP_COOKIE_JAR_LOC  = "location";
static const char *PROP_COOKIE_PERSIST  = "persistSessions";

static const char *PROP_PROXY           = "proxy";

Q_GLOBAL_STATIC(Config, globalConfig)

Config::Config() :
    m_doneInit(false),
    m_noCache(false),
    m_noCookieJar(false),
    m_maxCacheSize(-1)
{
}

void Config::init(QQmlEngine *engine)
{
    if (m_doneInit)
        return;

    m_doneInit = true;

    QNetworkAccessManager *network = engine->networkAccessManager();

    // Cache
    if (!m_noCache) {
        QNetworkDiskCache *cache = new QNetworkDiskCache();
        if (m_cachePath.isEmpty()) {
            m_cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                    + "/duperagent";
        }
        cache->setCacheDirectory(m_cachePath);
        if (m_maxCacheSize > 0)
            cache->setMaximumCacheSize(m_maxCacheSize);
        network->setCache(cache);
    }

    // Proxy
    if (m_systemProxy) {
        QNetworkProxyFactory::setUseSystemConfiguration(true);
    }

    // Cookies
    if (!m_noCookieJar) {
        if (m_cookieJarPath.isEmpty()) {
            m_cookieJarPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                + "/duperagent_cookies.txt";
        }
        CookieJar *cj = new CookieJar(m_cookieJarPath, network);
        if (m_persistSessionCookies)
            cj->setPersistSessions(m_persistSessionCookies);
        network->setCookieJar(cj);
    }
}

Config* Config::instance()
{
    Config *instance = globalConfig;
    return instance;
}

void Config::setOptions(const QJSValue &options)
{
    if (options.hasProperty(QString::fromLatin1(PROP_CACHE))) {
        QJSValue cacheOptions = options.property(QString::fromLatin1(PROP_CACHE));
        m_noCache = !cacheOptions.toBool();
        if (cacheOptions.hasProperty(QString::fromLatin1(PROP_CACHE_MAX_SIZE))) {
            m_maxCacheSize = cacheOptions.property(
                        QString::fromLatin1(PROP_CACHE_MAX_SIZE)).toUInt();
        }
        if (cacheOptions.hasProperty(QString::fromLatin1(PROP_CACHE_LOC))) {
            m_cachePath = cacheOptions.property(
                        QString::fromLatin1(PROP_CACHE_LOC)).toString();
        }
    }

    if (options.hasProperty(QString::fromLatin1(PROP_COOKIE_JAR))) {
        QJSValue jarOptions = options.property(QString::fromLatin1(PROP_COOKIE_JAR));
        m_noCookieJar = !jarOptions.toBool();
        if (jarOptions.hasProperty(QString::fromLatin1(PROP_COOKIE_JAR_LOC))) {
            m_cookieJarPath = jarOptions.property(
                        QString::fromLatin1(PROP_COOKIE_JAR_LOC)).toString();
        }
        if (jarOptions.hasProperty(QString::fromLatin1(PROP_COOKIE_PERSIST))) {
            m_persistSessionCookies = jarOptions.property(
                        QString::fromLatin1(PROP_COOKIE_PERSIST)).toBool();
        }
    }

    if (options.hasProperty(QString::fromLatin1(PROP_PROXY))) {
        QJSValue proxyOptions = options.property(QString::fromLatin1(PROP_PROXY));
        m_systemProxy = proxyOptions.toString() == QStringLiteral("system");
    }
}

} } }

