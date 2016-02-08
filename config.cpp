
#include <QtCore/QStandardPaths>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkAccessManager>
#include <QtQml/QQmlEngine>

#include "config.h"

namespace com { namespace cutehacks { namespace duperagent {

static const char *PROP_CACHE           = "cache";
static const char *PROP_CACHE_MAX_SIZE  = "maxSize";

Config::Config() :
    m_doneInit(false),
    m_noCache(false),
    m_maxCacheSize(-1)
{
}

void Config::init(QQmlEngine *engine)
{
    if (m_doneInit)
        return;

    if (!m_noCache) {
        QNetworkDiskCache *cache = new QNetworkDiskCache();
        QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                + "/duperagent";
        cache->setCacheDirectory(path);
        if (m_maxCacheSize > 0)
            cache->setMaximumCacheSize(m_maxCacheSize);
        engine->networkAccessManager()->setCache(cache);
    }
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
    }
}

} } }

