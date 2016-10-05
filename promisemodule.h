// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef PROMISEMODULE_H
#define PROMISEMODULE_H

#include <QtCore/QObject>
#include <QtQml/QJSValue>

#include "promise.h"

class QQmlEngine;

namespace com { namespace cutehacks { namespace duperagent {

class MultiPromiseExecutor : public QObject {
    Q_OBJECT

public:
    enum Behavior {
        All,
        Race
    };

    explicit MultiPromiseExecutor(QQmlEngine *, Behavior, QObject *parent = 0);
    QJSValue self() { return m_self; }

    Promise *addIterable(QJSValue);

public slots:
    void handlePromise(Promise::State, QJSValue);

protected:
    void promiseDone(Promise *);

private:
    QQmlEngine *m_engine;
    int m_index;
    int m_argsLength;
    Behavior m_behavior;
    QJSValue m_self;
    QJSValue m_args;
    Promise *m_promise;
};


class PromiseModule : public QObject
{
    Q_OBJECT

public:
    explicit PromiseModule(QQmlEngine *, QObject *parent = 0);

    Q_INVOKABLE QJSValue create(QJSValue);
    Q_INVOKABLE QJSValue all(QJSValue);
    Q_INVOKABLE QJSValue race(QJSValue);
    Q_INVOKABLE QJSValue resolve(QJSValue);
    Q_INVOKABLE QJSValue reject(QJSValue);

private:
    QQmlEngine *m_engine;
};

} } }

#endif // PROMISEMODULE_H
