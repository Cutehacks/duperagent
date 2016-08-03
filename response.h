// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef RESPONSE_H

#define RESPONSE_H
#include <QtCore/QObject>
#include <QtQml/QJSValue>

class QQmlEngine;
class QNetworkReply;

#include "qpm.h"

namespace com { namespace cutehacks { namespace duperagent {

class ResponsePrototype : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool info READ info)
    Q_PROPERTY(bool ok READ ok)
    Q_PROPERTY(bool clientError READ clientError)
    Q_PROPERTY(bool serverError READ serverError)
    Q_PROPERTY(bool error READ error)
    Q_PROPERTY(bool accepted READ accepted)
    Q_PROPERTY(bool noContent READ noContent)
    Q_PROPERTY(bool badRequest READ badRequest)
    Q_PROPERTY(bool unauthorized READ unauthorized)
    Q_PROPERTY(bool notAcceptable READ notAcceptable)
    Q_PROPERTY(bool notFound READ notFound)
    Q_PROPERTY(bool forbidden READ forbidden)

    Q_PROPERTY(bool fromCache READ fromCache)

    Q_PROPERTY(int status READ statusCode)
    Q_PROPERTY(int statusType READ statusType)
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(QString charset READ charset)
    Q_PROPERTY(QJSValue body READ body)
    Q_PROPERTY(QJSValue header READ header)

public:
    ResponsePrototype(QQmlEngine *, QNetworkReply *);
    ~ResponsePrototype();

    bool info() const;
    bool ok() const;
    bool clientError() const;
    bool serverError() const;
    bool error() const;
    bool accepted() const;
    bool noContent() const;
    bool badRequest() const;
    bool unauthorized() const;
    bool notAcceptable() const;
    bool notFound() const;
    bool forbidden() const;

    bool fromCache() const;

    int statusCode() const;
    int statusType() const;
    QString text() const;
    QString charset() const;
    QJSValue body() const;
    QJSValue header() const;

protected:
    bool typeEquals(int code) const;
    bool statusEquals(int code) const;

private:
    QQmlEngine *m_engine;
    QNetworkReply *m_reply;
    QString m_text;
    QString m_charset;
    QJSValue m_body;
    QJSValue m_header;
};

} } }

#endif // RESPONSE_H
