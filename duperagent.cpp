// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include <QtCore/QCoreApplication>
#include <QtQml/QQmlEngine>

#include "duperagent.h"
#include "request.h"
#include "config.h"
#include "cookiejar.h"
#include "promisemodule.h"
#include "networkactivityindicator.h"
#include "imageutils.h"

namespace com { namespace cutehacks { namespace duperagent {

static const char* DUPERAGENT_URI = "com.cutehacks.duperagent";

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

static QObject *promise_provider(QQmlEngine *engine, QJSEngine *)
{
    return new PromiseModule(engine);
}

static QObject *nai_provider(QQmlEngine *, QJSEngine *)
{
    return NetworkActivityIndicator::instance();
}

static QObject *iu_provider(QQmlEngine *engine, QJSEngine *)
{
    return new ImageUtils(engine);
}

static void registerTypes()
{
    qmlRegisterSingletonType<Request>(
        DUPERAGENT_URI,
        1, 0,
        "request",
        request_provider);

    qmlRegisterSingletonType<Request>(
        DUPERAGENT_URI,
        1, 0,
        "Request",
        request_provider);

    qmlRegisterSingletonType<PromiseModule>(
        DUPERAGENT_URI,
        1, 0,
        "promise",
        promise_provider);

    qmlRegisterSingletonType<PromiseModule>(
        DUPERAGENT_URI,
        1, 0,
        "Promise",
        promise_provider);

    qmlRegisterSingletonType<NetworkActivityIndicator>(
        DUPERAGENT_URI,
        1, 0,
        "NetworkActivityIndicator",
        nai_provider);

    qmlRegisterSingletonType<ImageUtils>(
        DUPERAGENT_URI,
        1, 0,
        "ImageUtils",
        iu_provider);

    qmlRegisterUncreatableType<CacheControl>(
        DUPERAGENT_URI,
        1, 0,
        "CacheControl",
        "Duperagent CacheControl enums.");

    qmlRegisterUncreatableType<ResponseType>(
                DUPERAGENT_URI,
                1, 0,
                "ResponseType",
                "Duperagent ResponseType enums.");

    qmlProtectModule(DUPERAGENT_URI, 1);
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)

} } }
