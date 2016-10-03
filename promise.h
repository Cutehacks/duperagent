// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef PROMISE_H
#define PROMISE_H

#include <QtCore/QObject>
#include <QtQml/QJSValue>

class QQmlEngine;

namespace com { namespace cutehacks { namespace duperagent {

class Promise;
struct Handler {
    QJSValue onFulfilled;
    QJSValue onRejected;
    Promise *next;
};

class Promise : public QObject
{
    Q_OBJECT

    enum State{
        PENDING,
        FULFILLED,
        REJECTED
    };

    Q_PROPERTY(bool isPending READ isPending)
    Q_PROPERTY(bool isFulfilled READ isFulfilled)
    Q_PROPERTY(bool isRejected READ isRejected)
    Q_PROPERTY(int length READ length)

public:
    explicit Promise(QQmlEngine *, QJSValue = QJSValue());
    inline QJSValue self() {            return m_self;}

    Q_INVOKABLE QJSValue then(QJSValue = QJSValue(), QJSValue = QJSValue());
    Q_INVOKABLE void fulfill(const QJSValue&);
    Q_INVOKABLE void reject(const QJSValue&);

    inline bool isPending() const {     return m_state == PENDING;}
    inline bool isFulfilled() const {   return m_state == FULFILLED;}
    inline bool isRejected() const {    return m_state == REJECTED;}
    inline int length() const {         return 1; }

protected:
    void call(QJSValue, const QJSValue&, Promise *);
    void merge(Promise *);

signals:
    void settled(State, QJSValue);

private slots:
    void settle(State, QJSValue);

private:
    QQmlEngine *m_engine;
    QJSValue m_self;
    State m_state;
    QJSValue m_value;
    QList<Handler> m_handlers;
};

} } }

#endif // PROMISE_H
