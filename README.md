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
* Promise API
* Secure connection (SSL/TLS) API
* Network activity indicator
* Image utilities

# Limitations

* DuperAgent requires Qt 5.4.
* DuperAgent does not implement the XML API of XmlHttpRequest

# Migration from v0.x.x to v1.x.x
 
There have been no API removals in the 1.x release so code written using v0.x should continue to
work without modification.

There is however one major behavioral change. v1.x enables a cookie jar by default such that responses
containing cookies will send those cookies with applicable requests. This may introduce a security issue
for some applications, so to disable this behavior and revert to v0.x behavior, call this 
function somewhere during initialization before making any requests:

```
    Http.Request.config({
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

    Http.Request
        .get("http://httpbin.org/get")
        .timeout(5000)
        .end(function(err, res) {
            console.log(res.status);
            console.log(JSON.stringify(res.header, null, 4));
            console.log(JSON.stringify(res.body, null, 4));
        });

```

You can of course use whatever you want instead of `Http`, but this documentation uses this alias throughout.

# Request API

At this point in time, the API should be almost identical to that of SuperAgent so that documentation
is recommended. The following functions are not yet implemented in DuperAgent:

- `withCredentials`: CORS - Not implemented
- `buffer`: Response body buffering is not yet implemented
- `pipe`: Piping to streams is not supported

That being said, the following API additions are also available:

## config()

A function for setting global configuration options for the agent. This function should be called
once before any requests are made. It is good practice to call this from a `Component.onCompleted`
signal. For example:

```
    Component.onCompleted: {
        Http.Request.config({
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
    Http.Request.config({
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
    Http.Request.config({
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
    Http.Request.config({
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

## clearCookies

This function will clear all saved cookies including those marked as *HttpOnly*.

# Promise API

This package contains an implementation of the [Promises/A+](https://promisesaplus.com/) specification and also 
offers an API similar to the Promises API in ES2017. For more information, please see 
[MDN](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise)

At some point this API may be split out into it's own module.

## create

Creates a new Promise object.

Since the Javascript engine in QML does not allow us to expose new types with constructors, the
`new Promise(executor)` syntax is not supported. Instead the create function can be used like so:

```js
var p = Http.Promise.create(function(resolve, reject) {

    asynchronousStuff(function(success) {
        if (success) {
            resolve("worked!");
        } else {
            reject("something failed");
        }
    });

});

p1.then(function(value) {
    // success
}).catch(function(reason) {
    // failure
});
```

## resolve

Returns a resolved promise.

```js
var p = Http.Promise.resolve(5);

p.then(function(value) {
    console.log(value); // 5
});
```

## reject

Returns a rejected promise.

```js
var p = Http.Promise.reject("error");

p.catch(function(reason) {
    console.log(reason); // "error"
});
```

## all

Concurrently executes several promises and returns another promise that is fulfilled 
when the original promises are fulfilled. The resolved value is an array containing
the values of the original promises in the original order (not the order they were
fulfilled).

If one of the original promises fail, the returned promise will be rejected with the 
same reason.

```js
var p1 = Http.Promise.resolve(3);
var p2 = 1337;
var p3 = Http.Request
        .get("http://httpbin.org/get")
        .then(function(resp) {
            return resp.body;
        });

var p4 = Http.Promise.all([p1, p2, p3]);

p4.then(function(values) {
    console.log(values[0]); // 3
    console.log(values[1]); // 1337
    console.log(values[2]); // resp.body
});
```

## race

Concurrently executes several promises and returns another promise that is fulfilled 
when the first of those promises is fulfilled.

```js
var p1 = Http.Request
     .get("http://httpbin.org/delay/3")
     .then(function(resp) {
         return 3;
     });

var p2 = Http.Request
    .get("http://httpbin.org/delay/1")
    .then(function(resp) {
        return 1;
    });

var p3 = Http.Promise.race([p1, p2]);

p3.then(function(value) {
    console.log(value); // 1
});
```

# NetworkActivityIndicator API

The network activity indicator item exposes properties that can be used 
to show when there are outstanding network requests in the application. There
is a native implementation for iOS and there is also a generic implementation
that works on all platforms (including iOS).

Only network request created through Duperagent are considered.

## enabled : bool

A read-only property that is `true` when Duperagent has a non-zero number
of pending network requests. It can be used to show a busy indicator or some
other visual element to indicate that something is happening.

## activationDelay : int

Time (in ms) to wait before enabling the network activity indicator after new
requests have been initiated. The default is `1000` ms.

## completionDelay : int

Time (in ms) to wait before disabling the network activity indicator after all
requests have completed. The default is `170` ms.

## enableNativeIndicator : bool

A property that indicates if the native implementation of a network activity
indicator should be used. This feature is only available on iOS. The default
value of this property is `false`.

# ImageUtils API

The ImageUtils module provides functionality for scaling and cropping an
image file before uploading it via DuperAgent's `attach` function. The first
step is to create a reader and then get and set properties on the reader
before calling `read` to load the image with the given settings.

## createReader

This function accepts a file path to an image or to a base64 encoded data
uri containing image data and returns an image reader. The image is *not*
decoded at this point, only the reader is created.

## size

Returns an object with the `width` and `height` properties of the image without
actually decoding the image.

## fileSize

Returns the filesize (in bytes) of the encoded image.

## setScaledSize

Sets the desired scaled size for the image when it is decoded. Depending on
the image format, some decoders can do this efficiently without having
to decode the full image first and then downscale.

```js
var reader = Http.ImageUtils.createReader(imagePath);
reader.setScaledSize(200, 200, Image.PreserveAspectCrop);
reader.read();
```

*NOTE*: Only the first 3 enumerations of `Image.fitMode` are supported in the
final (optional) argument to the function. If the third argument is omitted,
it defaults to `Image.Stretch`.

## setClipRect

Sets the clip rect for image reader.

```js
var reader = Http.ImageUtils.createReader(imagePath);
reader.setClipRect(0, 0, 200, 200);
reader.read();
```

See also QImageReader::setClipRect.

## setScaledClipRect

Sets a scaled clip rect for image reader.

```js
var reader = Http.ImageUtils.createReader(imagePath);
reader.setScaledClipRect(0, 0, 200, 200);
reader.read();
```

See also QImageReader::setScaledClipRect.

## setAutoTransform

Rotates the image if the metadata for the image indicates that it should.

```js
Http.ImageUtils.createReader(imagePath)
    .setAutoTransform(true)
    .setScaledSize(200, 200)
    .read();
```

*NOTE*: This function has no effect on versions of Qt prior to Qt 5.5.0.

##  read

The function that actually reads and decodes the image into memory based on
the settings previously configured on the reader. This function takes an
optional object of options which can transcode the image to a different format
and quality.

```js
Http.ImageUtils.createReader(imagePath)
    .setScaledSize(200, 200)
    .read({
        transcode: {
            format: "jpg",
            quality: 50
        }
    });
```

The object returned from this function can be passed to the `attach`
function of the `request` type as the second argument.
