// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef DUPERAGENT_H
#define DUPERAGENT_H

#include <QtCore/QObject>
#include <QtQml/QJSValue>
#include <QtQml/qqml.h>

#include "qpm.h"

class QQmlEngine;
class QJsEngine;

namespace com { namespace cutehacks { namespace duperagent {

class ResponseType : public QObject {
    Q_OBJECT
    Q_ENUMS(Types)
public:
    enum Types {
        Auto        = 0,
        Text        = 1,
        Json        = 2,
        Blob        = 3,
        ArrayBuffer = 4
    };
};

class Request : public QObject
{
    Q_OBJECT

public:
    Request(QQmlEngine *engine, QObject *parent = 0);

    Q_PROPERTY(QJSValue cookie READ cookie WRITE setCookie)

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

    QJSValue cookie() const;
    void setCookie(const QJSValue &);

    Q_INVOKABLE void clearCookies();

private:
    QQmlEngine *m_engine;
};


void registerTypes();

} } }

#endif // DUPERAGENT_H
