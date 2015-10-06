#ifndef CONFIG_H
#define CONFIG_H

#include <QGlobalStatic>
#include <QtQml/QJSValue>

#include "qpm.h"

class QQmlEngine;

QPM_BEGIN_NAMESPACE(com, cutehacks, duperagent)

class Config
{
public:
    Config();
    void init(QQmlEngine *);
    void setOptions(const QJSValue&);

private:
    bool m_doneInit;
    bool m_noCache;
    QString m_cachePath;
    qint64 m_maxCacheSize;
};

Q_GLOBAL_STATIC(Config, getConfig)

QPM_END_NAMESPACE(com, cutehacks, duperagent)

#endif // CONFIG_H
