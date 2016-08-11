// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QCoreApplication>
#include <QtQml/QQmlEngine>

#include "duperagent.h"
#include "request.h"
#include "config.h"
#include "cookiejar.h"

namespace com { namespace cutehacks { namespace duperagent {

extern ContentTypeMap contentTypes;

Request::Request(QQmlEngine *engine, QObject *parent) :
    QObject(parent), m_engine(engine)
{
    contentTypes.insert("html", "text/html");
    contentTypes.insert("json", "application/json");
    contentTypes.insert("xml", "application/xml");
    contentTypes.insert("form", "application/x-www-form-urlencoded");
    contentTypes.insert("form-data", "application/x-www-form-urlencoded");
}

void Request::config(const QJSValue &options)
{
    Config::instance()->setOptions(options);
}

QJSValue Request::get(const QJSValue &url, const QJSValue &data, const QJSValue &fn) const
{
    RequestPrototype *proto = new RequestPrototype(
                m_engine,
                RequestPrototype::Get,
                QUrl(url.toString()));

    if (data.isCallable()) {
        proto->end(data);
    } else if (!data.isUndefined()){
        proto->query(data);
    }

    if (fn.isCallable()) {
        proto->end(fn);
    }

    return proto->self();
}

QJSValue Request::head(const QJSValue &url, const QJSValue &data, const QJSValue &fn) const
{
    RequestPrototype *proto = new RequestPrototype(
                m_engine,
                RequestPrototype::Head,
                QUrl(url.toString()));

    if (data.isCallable()) {
        proto->end(data);
    } else if (!data.isUndefined()){
        proto->send(data);
    }

    if (fn.isCallable()) {
        proto->end(fn);
    }

    return proto->self();
}

QJSValue Request::del(const QJSValue &url, const QJSValue &fn) const
{
    RequestPrototype *proto = new RequestPrototype(
                m_engine,
                RequestPrototype::Delete,
                QUrl(url.toString()));

    if (fn.isCallable()) {
        proto->end(fn);
    }

    return proto->self();
}

QJSValue Request::patch(const QJSValue &url, const QJSValue &data, const QJSValue &fn) const
{
    RequestPrototype *proto = new RequestPrototype(
                m_engine,
                RequestPrototype::Patch,
                QUrl(url.toString()));

    if (data.isCallable()) {
        proto->end(data);
    } else if (!data.isUndefined()){
        proto->send(data);
    }

    if (fn.isCallable()) {
        proto->end(fn);
    }

    return proto->self();
}

QJSValue Request::post(const QJSValue &url, const QJSValue &data, const QJSValue &fn) const
{
    RequestPrototype *proto = new RequestPrototype(
                m_engine,
                RequestPrototype::Post,
                QUrl(url.toString()));

    if (data.isCallable()) {
        proto->end(data);
    } else if (!data.isUndefined()){
        proto->send(data);
    }

    if (fn.isCallable()) {
        proto->end(fn);
    }

    return proto->self();
}

QJSValue Request::put(const QJSValue &url, const QJSValue &data, const QJSValue &fn) const
{
    RequestPrototype *proto = new RequestPrototype(
                m_engine,
                RequestPrototype::Put,
                QUrl(url.toString()));

    if (data.isCallable()) {
        proto->end(data);
    } else if (!data.isUndefined()){
        proto->send(data);
    }

    if (fn.isCallable()) {
        proto->end(fn);
    }

    return proto->self();
}

QJSValue Request::cookie() const
{
    Config::instance()->init(m_engine);

    CookieJar *jar = qobject_cast<CookieJar*>(m_engine->networkAccessManager()->cookieJar());
    if (!jar)
        return QJSValue("");

    return QJSValue(jar->cookies());
}

void Request::setCookie(const QJSValue &cookie)
{
    Config::instance()->init(m_engine);

    CookieJar *jar = qobject_cast<CookieJar*>(m_engine->networkAccessManager()->cookieJar());
    if (!jar)
        return;

    jar->addCookie(cookie.toString());
}

void Request::clearCookies()
{
    Config::instance()->init(m_engine);

    CookieJar *jar = qobject_cast<CookieJar*>(m_engine->networkAccessManager()->cookieJar());
    if (!jar)
        return;

    jar->clearAll();
}

static QObject *request_provider(QQmlEngine *engine, QJSEngine *)
{
    return new Request(engine);
}

static void registerTypes()
{
    qmlRegisterSingletonType<Request>(
        "com.cutehacks.duperagent",
        1, 0,
        "request",
        request_provider);
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)

} } }
