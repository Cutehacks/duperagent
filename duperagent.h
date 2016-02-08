// Copyright 2015 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef DUPERAGENT_H
#define DUPERAGENT_H

#include <QtCore/QObject>
#include <QtQml/QJSValue>
#include <QtQml/qqml.h>

#include "qpm.h"

class QQmlEngine;
class QJsEngine;

QPM_BEGIN_NAMESPACE(com, cutehacks, duperagent)

class Request : public QObject
{
    Q_OBJECT

public:
    Request(QQmlEngine *engine, QObject *parent = 0);

    Q_INVOKABLE void config(const QJSValue &);

    Q_INVOKABLE QJSValue get(const QJSValue&,
                              const QJSValue& = QJSValue(),
                              const QJSValue& = QJSValue()) const;
    Q_INVOKABLE QJSValue head(const QJSValue&,
                              const QJSValue& = QJSValue(),
                              const QJSValue& = QJSValue()) const;
    Q_INVOKABLE QJSValue del(const QJSValue&,
                              const QJSValue& = QJSValue()) const;
    Q_INVOKABLE QJSValue patch(const QJSValue&,
                              const QJSValue& = QJSValue(),
                              const QJSValue& = QJSValue()) const;
    Q_INVOKABLE QJSValue post(const QJSValue&,
                              const QJSValue& = QJSValue(),
                              const QJSValue& = QJSValue()) const;
    Q_INVOKABLE QJSValue put(const QJSValue&,
                              const QJSValue& = QJSValue(),
                              const QJSValue& = QJSValue()) const;

private:
    QQmlEngine *m_engine;
};

static QObject *request_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine)
    return new Request(engine);
}

static void registerEngine(QQmlEngine *)
{
    qmlRegisterSingletonType<Request>(
        "com.cutehacks.duperagent",
        1, 0,
        "request",
        request_provider);
}

QPM_END_NAMESPACE(com, cutehacks, duperagent)

#endif // DUPERAGENT_H
