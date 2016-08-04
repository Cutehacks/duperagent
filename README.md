[![Build Status](https://travis-ci.org/Cutehacks/duperagent.svg?branch=master)](https://travis-ci.org/Cutehacks/duperagent)

DuperAgent is a QML clone of the fantastic [SuperAgent](https://visionmedia.github.io/superagent)
library from VisionMedia.

The library is implemented in Qt/C++ and designed as an alternative to QML's builtin implementation
of XmlHttpRequest.

# Features

* Modern API in comparison to XmlHttpRequest
* Support for multipart/form uploads
* Automatic parsing of response bodies with known content-types
* Built-in persistent cookie jar

# Limitations

* DuperAgent requires Qt 5.2.
* DuperAgent does not implement the XML API of XmlHttpRequest

# Migration from v0.x.x to v1.x.x
 
There have been no API removals in the 1.x release so code written using v0.x should continue to
work without modification.

There is however one major behavioral change. v1.x enables a cookie jar by default such that responses
containing cookies will send those cookies with applicable requests. This may introduce a security issue
for some applications, so to disable this behavior and revert to v0.x behavior, call this 
function somewhere during initialization before making any requests:

```
    Http.request.config({
        cookieJar: false
    });
```

# Installation

## With qpm

Duperagent is available on qpm.io and can easily be installed like so:

`qpm install com.cutehacks.duperagent`

Adding the following line to your .pro file will pull in all of your qpm dependencies, including DuperAgent.

`include(vendor/vendor.pri)`

## Without qpm

Alternatively you can clone the repository directly and include it in your project:

`include(path/to/com_cutehacks_duperagent.pri)`

# Usage

Once the files are compiled into your binary, DuperAgent will register it's types with the QMLEngine automatically.
Import the module and start using it inside your .qml files:

```
import com.cutehacks.duperagent 1.0 as Http

...

    Http.request
        .get("http://httpbin.org/get")
        .timeout(5000)
        .end(function(err, res) {
            console.log(res.status);
            console.log(JSON.stringify(res.header, null, 4));
            console.log(JSON.stringify(res.body, null, 4));
        });

```

You can of course use whatever you want instead of `Http`, but this documentation uses this alias throughout.

# API

At this point in time, the API should be almost identical to that of SuperAgent so that documentation
is recommended. The following functions are not yet implemented in DuperAgent:

- `withCredentials`: CORS - Not implemented yet
- `then`: ES6 style promises are not yet implemented
- `buffer`: Response body buffering is not yet implemented
- `pipe`: Piping to streams is not supported

That being said, the following API additions are also available:

## config()

A function for setting global configuration options for the agent. This function should be called
once before any requests are made. It is good practice to call this from a `Component.onCompleted`
signal. For example:

```
    Component.onCompleted: {
        Http.request.config({
            cache: false
        });
    }
```

The following is a list of supported configuration options:

### `cache`

This option controls the cache behavior of the agent. The default is to create a QNetworkDiskCache with
a sensible path for the platform. If you wish to disable this behavior, for example because you have your
own cache, you can set this property to `false`. The property can be further customized by passing an object.

```
    Http.request.config({
        cache: {
            maxSize: 20000,
            location: "/path/to/cache"
        }
    });
```

* `maxSize`: The maximum size of the cache
* `location`: The full path to the directory for caching files. The default is `<CacheLocation>/duperagent`

### `cookieJar`

This option controls the cookie jar of the agent. The default is to create an instance of a QNetworkCookieJar
that additionally persists the cookies to disk. If you wish to disable this behavior, for example because you
have your own cookie jar, you can set this property to `false`. This property can be further customized by
passing an object.

```
    Http.request.config({
        cookieJar: {
            location: "/path/to/cookies.txt"
        }
    });
```

* `location`: The full path to the file to use for the disk storage. The default is `<AppDataLocation>/duperagent_cookies.txt`

### `proxy`

This option controls the proxy settings used by agent. By default Qt does not use a proxy, however you can use
the system proxy by setting this value to `"system"`.

```
    Http.request.config({
        proxy: "system"
    });
```

## cookie

This function behaves similar to `document.cookie` as implemented in browsers.

Calling the property as a getter will return all cookies in the cookie jar as a semi-colon 
separated string. Note that this function will not return cookies that are marked as *HttpOnly*.

Calling the property as a setter will add the cookie to the cookie jar. If there is an existing
cookie with the same identifying tuple in the jar, it will overwrite it. Note that it is not
possible to overwrite cookies marked as *HttpOnly*. Attempts to overwrite these will be silently ignored.
