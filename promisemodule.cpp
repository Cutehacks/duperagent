// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtQml/QQmlEngine>
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
#include "jsvalueiterator.h"
#else
#include <QtQml/QJSValueIterator>
typedef QJSValueIterator JSValueIterator;
#endif

#include "promisemodule.h"
#include "promise.h"

namespace com { namespace cutehacks { namespace duperagent {

static const char* INDEX_PROPERTY = "value_index";

PromiseModule::PromiseModule(QQmlEngine *engine, QObject *parent) :
    QObject(parent),
    m_engine(engine)
{ }

QJSValue PromiseModule::create(QJSValue executor)
{
    Promise *p = new Promise(m_engine, executor);
    return p->self();
}

QJSValue PromiseModule::all(QJSValue iterable)
{
    if (iterable.isArray()) {
        MultiPromiseExecutor *executor = new MultiPromiseExecutor(
            m_engine,
            MultiPromiseExecutor::All);

        return executor->addIterable(iterable)->self();
    } else {
        return reject("Argument passed to Promise.all was not iterable");
    }
}

QJSValue PromiseModule::race(QJSValue iterable)
{
    if (iterable.isArray()) {
        MultiPromiseExecutor *executor = new MultiPromiseExecutor(
            m_engine,
            MultiPromiseExecutor::Race);

        return executor->addIterable(iterable)->self();
    } else {
        return reject("Argument passed to Promise.all was not iterable");
    }
}

QJSValue PromiseModule::resolve(QJSValue value)
{
    Promise *p = new Promise(m_engine);
    p->fulfill(value);
    return p->self();
}

QJSValue PromiseModule::reject(QJSValue reason)
{
    Promise *p = new Promise(m_engine);
    p->reject(reason);
    return p->self();
}

MultiPromiseExecutor::MultiPromiseExecutor(QQmlEngine *engine, Behavior b, QObject *parent) :
    QObject(parent),
    m_engine(engine),
    m_index(0),
    m_argsLength(0),
    m_behavior(b),
    m_promise(0)
{
    m_self = engine->newQObject(this);
}

Promise *MultiPromiseExecutor::addIterable(QJSValue iterable)
{
    m_promise = new Promise(m_engine);
    m_argsLength = iterable.property("length").toInt();
    m_args = m_engine->newArray(m_argsLength);

    JSValueIterator it(iterable);
    while (it.next()) {
        if (!it.hasNext()) //last item is length
            break;

        Promise *p = qobject_cast<Promise*>(it.value().toQObject());
        if (!p) {
            p = new Promise(m_engine);
            p->fulfill(it.value());
        }

        p->setProperty(INDEX_PROPERTY, m_index++);

        if (p->isPending()) {
            connect(p, SIGNAL(settled(Promise::State, QJSValue)),
                    this, SLOT(handlePromise(Promise::State, QJSValue)));
        } else {
            promiseDone(p);
        }
    }

    return m_promise;
}

void MultiPromiseExecutor::handlePromise(Promise::State, QJSValue)
{
    Promise *p = qobject_cast<Promise*>(sender());
    if (!p)
        return;

    promiseDone(p);
}

void MultiPromiseExecutor::promiseDone(Promise *p)
{
    if (m_behavior == MultiPromiseExecutor::All) {
        if (p->isRejected()) {
            m_promise->reject(p->value());
            blockSignals(true); // stop listening
        } else {
            bool ok;
            int index = p->property(INDEX_PROPERTY).toInt(&ok);

            if (!ok)
                return; // todo: log error?

            m_args.setProperty(index, p->value());

            if (--m_argsLength == 0)
                m_promise->fulfill(m_args);
        }
    } else /* MultiPromiseExecutor::Race */ {
        if (p->isRejected()) {
            m_promise->reject(p->value());
        } else {
            m_promise->fulfill(p->value());
        }
        blockSignals(true); // stop listening
    }
}

} } }
