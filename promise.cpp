// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtQml/QQmlEngine>

#include "promise.h"

namespace com { namespace cutehacks { namespace duperagent {

Promise::Promise(QQmlEngine *engine, QJSValue executor) :
    QObject(0),
    m_engine(engine),
    m_state(PENDING)
{
    m_self = m_engine->newQObject(this);
    m_engine->setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);

    if (executor.isCallable()) {
        executor.call(QJSValueList()
                      << m_self.property("fulfill")
                      << m_self.property("reject"));
    }
}

QJSValue Promise::then(QJSValue onFulfilled, QJSValue onRejected)
{
    Promise *next = new Promise(m_engine);

    if (m_state == FULFILLED) {
        call(onFulfilled, m_value, next);
    } else if (m_state == REJECTED) {
        call(onRejected, m_value, next);
    } else {
        Handler h = {
            onFulfilled,
            onRejected,
            next,
        };
        m_handlers.append(h);
    }

    return next->self();
}

void Promise::endCallback(const QJSValue &err, const QJSValue &res)
{
    if (err.isError()) {
        reject(err);
    } else {
        fulfill(res);
    }
}

void Promise::fulfill(const QJSValue &value)
{
    if (m_state != PENDING)
        return;

    m_state = FULFILLED;
    m_value = value;

    while (!m_handlers.empty()) {
        Handler h = m_handlers.takeFirst();
        call(h.onFulfilled, m_value, h.next);
    }

    emit settled(m_state, m_value);
}

void Promise::reject(const QJSValue &reason)
{
    if (m_state != PENDING)
        return;

    m_state = REJECTED;
    m_value = reason;

    while (!m_handlers.empty()) {
        Handler h = m_handlers.takeFirst();
        call(h.onRejected, m_value, h.next);
    }

    emit settled(m_state, m_value);
}

void Promise::call(QJSValue fn, const QJSValue& arg, Promise *promise)
{
    Q_ASSERT(m_state != PENDING);

    if (!fn.isCallable()) {
        if (m_state == FULFILLED)
            promise->fulfill(fn);
        else
            promise->reject(fn);
    } else {
        QJSValue result = fn.call(QJSValueList() << arg);
        if (result.isError()) {
            promise->reject(result);
        } else {
            Promise *p = qobject_cast<Promise*>(result.toQObject());
            if (p) {
                if (p == promise) {
                    // 2.3.1 If promise and x refer to the same object, reject
                    // promise with a TypeError as the reason.
                    promise->reject(m_engine->evaluate("new TypeError();"));
                } else {
                    // 2.3.2 If x is a promise, adopt its state
                    promise->merge(p);
                }
            } else if (result.isCallable() || result.isObject()) {
                // 2.3.3 Otherwise, if x is an object or function,

                // 2.3.3.1 Let then be x.then.
                QJSValue then = result.property("then");

                if (then.isError()) {
                    // 2.3.3.2 If retrieving the property x.then results in a
                    // thrown exception e, reject promise with e as the reason.
                    promise->reject(then);
                } else if (then.isCallable()) {
                    // 2.3.3.3 If then is a function, call it with x as this,
                    // first argument resolvePromise, and second argument
                    // rejectPromise
                    QJSValue resolvePromise = promise->self().property("fulfill");
                    QJSValue rejectPromise = promise->self().property("reject");

                    QJSValue thenResult = then.callWithInstance(result,
                        QJSValueList() << resolvePromise << rejectPromise);

                    if (thenResult.isError()) {
                        promise->reject(thenResult);
                    }
                } else {
                    // 2.3.3.4 If then is not a function, fulfill promise with x.
                    promise->fulfill(result);
                }
            } else {
                // 2.3.4 If x is not an object or function, fulfill promise with x.
                promise->fulfill(result);
            }
        }
    }
}

void Promise::merge(Promise *other)
{
    if (other->m_state != PENDING) {
        // 2.3.2.2 If/when x is fulfilled, fulfill promise with the same value.
        // 2.3.2.3 If/when x is rejected, reject promise with the same reason.
        settle(other->m_state, other->m_value);
    } else {
        // 2.3.2.1 If x is pending, promise must remain pending until x is
        // fulfilled or rejected.
        connect(other, SIGNAL(settled(State, QJSValue)),
                this, SLOT(settle(State, QJSValue)));
    }
}

void Promise::settle(Promise::State state, QJSValue value)
{
    if (state == FULFILLED) {
        fulfill(value);
    } else {
        reject(value);
    }
}


} } }
