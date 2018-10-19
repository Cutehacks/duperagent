// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

import QtQuick 2.3
import QtTest 1.0

import com.cutehacks.duperagent 1.0 as Http

TestCase {
    id: test
    name: "httpbin"
    property int timeout: 6000
    signal done

    function initTestCase() {
        Http.Request.config({
            proxy: "system" // for debugging with Charles Proxy
        });
        Http.Request.clearCookies();
    }

    function init() {
        async.clear();
    }

    function expiresNextYear() {
        var d = new Date();
        d.setUTCFullYear(d.getUTCFullYear() + 1);
        return d.toUTCString();
    }

    SignalSpy {
        id: async
        target: test
        signalName: "done"
    }

    function test_get() {
        Http.Request
            .get("http://httpbin.org/get")
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(res.body.url, "http://httpbin.org/get")
                done();
            });

        async.wait(timeout);
    }

    function test_post_json() {
        Http.Request
            .post("http://httpbin.org/post")
            .send({foo: "bar"})
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(res.body.json.foo, "bar");
                done();
            });

        async.wait(timeout);
    }

    function test_post_json_array() {
        Http.Request
        .post("http://httpbin.org/post")
        .send(["foo", "bar", "baz"])
        .end(function(err, res){
            verify(!err, err);
            compare(res.status, 200);
            compare(Object.prototype.toString.call(res.body.json), "[object Array]")
            compare(res.body.json[0], "foo");
            compare(res.body.json[1], "bar");
            compare(res.body.json[2], "baz");
            done();
        });

        async.wait(timeout);
    }

    function test_post_form() {
        Http.Request
            .post("http://httpbin.org/post")
            .type("form")
            .send({foo: "bar"})
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(res.body.form.foo, "bar");
                done();
            });

        async.wait(timeout);
    }

    function test_put() {
        Http.Request
            .put("http://httpbin.org/put")
            .send({foo: "bar"})
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                done();
            });

        async.wait(timeout);
    }

    function test_patch() {
        Http.Request
            .patch("http://httpbin.org/patch")
            .send({foo: "bar"})
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                done();
            });

        async.wait(timeout);
    }

    function test_delete() {
        Http.Request
            .del("http://httpbin.org/delete")
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                done();
            });

        async.wait(timeout);
    }

    function test_gzip() {
        Http.Request
            .get("http://httpbin.org/gzip")
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(res.body.gzipped, true);
                done();
            });

        async.wait(timeout);
    }

    function test_deflate() {
        Http.Request
            .get("http://httpbin.org/deflate")
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(res.body.deflated, true);
                done();
            });

        async.wait(timeout);
    }

    function test_404() {
        Http.Request
            .get("http://httpbin.org/status/404")
            .end(function(err, res){
                verify(err);
                compare(res.status, 404);
                done();
            });

        async.wait(timeout);
    }

    function test_redirect_5() {
        Http.Request
            .get("http://httpbin.org/redirect/5")
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                done();
            });

        async.wait(timeout);
    }

    function test_redirect_7_fail() {
        Http.Request
        .get("http://httpbin.org/redirect/7")
        .end(function(err, res){
            verify(err);
            compare(res.status, 302);
            done();
        });

        async.wait(timeout);
    }

    function test_redirect_7_pass() {
        Http.Request
        .get("http://httpbin.org/redirect/7")
        .redirects(7)
        .end(function(err, res){
            verify(!err, err);
            compare(res.status, 200);
            done();
        });

        async.wait(timeout);
    }

    function test_basic_auth() {
        Http.Request
            .get("http://httpbin.org/basic-auth/username/password")
            .auth("username", "password")
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(res.body.authenticated, true);
                done();
            });

        async.wait(timeout);
    }

    function test_basic_auth_badpass() {
        Http.Request
            .get("http://httpbin.org/basic-auth/username/password")
            .auth("username", "notthepassword")
            .end(function(err, res){
                verify(err);
                compare(res.status, 401);
                done();
            });

        async.wait(timeout);
    }


    function test_useragent() {
        var ua = "Duperagent 1.0";
        Http.Request
            .get("http://httpbin.org/user-agent")
            .set("user-agent", ua)
            .end(function(err, res){
                verify(!err);
                compare(res.status, 200);
                compare(res.body["user-agent"], ua)
                done();
            });

        async.wait(timeout);
    }

    function test_cookies() {
        Http.Request.cookie = "food=pizza; domain=httpbin.org";

        Http.Request
            .get("http://httpbin.org/cookies/set")
            .query({foo: "bar"})
            .end(function(err, res){
                compare(res.body.cookies.foo, "bar")
                compare(res.body.cookies.food, "pizza")
                done();
            });

        async.wait(timeout);
    }

    function test_cookies_persistent() {
        Http.Request.cookie = "persistent=for1year; domain=httpbin.org; expires=" + expiresNextYear();

        Http.Request
            .get("http://httpbin.org/cookies/set")
            .query({foo: "bar"})
            .end(function(err, res){
                verify(res.body.cookies.persistent)
                verify(Http.Request.cookie !== "");
                done();
            });

        async.wait(timeout);
    }

    function test_cookies_delete() {
        Http.Request.cookie = "victim=me; domain=httpbin.org; path=/; expires=" + expiresNextYear();

        Http.Request
            .get("http://httpbin.org/cookies/delete?victim=")
            .end(function(err, res){
                verify(!res.body.cookies.victim)
                verify(!Http.Request.cookie || Http.Request.cookie.indexOf("victim") < 0);
                done();
            });

        async.wait(timeout);
    }

    function test_cache() {
        Http.Request
            .get("http://httpbin.org/cache/3")
            .end(function(err, res){
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
        async.clear();

        Http.Request
            .get("http://httpbin.org/cache/3")
            .end(function(err, res){
                verify(res.fromCache);
                done();
            });

        async.wait(timeout);
        async.clear();

        sleep(5000);

        Http.Request
            .get("http://httpbin.org/cache/3")
            .end(function(err, res){
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
    }

    function test_cache_load_always_cache() {
        sleep(5000);

        Http.Request
            .get("http://httpbin.org/cache/3")
            .cacheSave(true)
            .end(function(err, res){
                verify(!err);
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
        async.clear();

        sleep(5000);

        Http.Request
            .get("http://httpbin.org/cache/3")
            .cacheLoad(Http.CacheControl.AlwaysCache)
            .end(function(err, res){
                verify(res.fromCache);
                done();
            });

        async.wait(timeout);
    }

    function test_cache_load_always_network() {
        sleep(5000);

        Http.Request
            .get("http://httpbin.org/cache/3")
            .cacheSave(true)
            .end(function(err, res){
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
        async.clear();

        Http.Request
            .get("http://httpbin.org/cache/3")
            .cacheLoad(Http.CacheControl.AlwaysNetwork)
            .end(function(err, res){
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
    }

    function test_cache_save() {
        sleep(5000);

        Http.Request
            .get("http://httpbin.org/cache/3")
            .cacheSave(true)
            .end(function(err, res){
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
        async.clear();

        Http.Request
            .get("http://httpbin.org/cache/3")
            .end(function(err, res){
                verify(res.fromCache);
                done();
            });

        async.wait(timeout);
        async.clear();

        sleep(5000);

        Http.Request
            .get("http://httpbin.org/cache/3")
            .cacheSave(false)
            .end(function(err, res){
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
        async.clear();

        Http.Request
            .get("http://httpbin.org/cache/3")
            .end(function(err, res){
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
    }

    function test_multipart() {
        Http.Request
            .post("http://httpbin.org/post")
            .field('foo', 'bar')
            .attach('txt', ':/data.txt')
            .end(function(err, res){
                compare(res.body.form.foo, "bar");
                compare(res.body.files.txt, "WORKED!\n");
                done();
            });

        async.wait(10000);
    }

    function test_events() {
        var events = {
            request: false,
            progress: false,
            end: false,
            response: false
        };

        Http.Request
            .get("http://httpbin.org/bytes/" + 2 * 1024 * 1024)
            .on('request', function(req) {
                events.request = req;
            })
            .on('progress', function(progress) {
                events.progress = progress;
            })
            .on('end', function() {
                events.end = true;
            })
            .on('progress', function(res) {
                events.response = res;
            })
            .end(function(err, res){
                verify(events.request);
                verify(events.progress);
                verify(events.end);
                verify(events.response);
                done();
            });

        async.wait(60000);
    }

    function test_use() {
        var setHeader = function(req) {
            req.headers["X-Custom-Header"] =  "1"
            return req
        }

        var setUrl = function(req) {
            req.url = "http://httpbin.org" + req.url
            return req
        }

        Http.Request
            .get("/get")
            .use(setHeader)
            .use(setUrl)
            .end(function(err, res) {
                compare(res.body.headers["X-Custom-Header"], "1")
                done();
            })

        async.wait(timeout);
    }

    function test_then_pending() {
        Http.Request
            .get("http://httpbin.org/get")
            .then(function(resp) {
                done();
                return resp;
            });
        async.wait(timeout);
    }

    function test_then_rejected() {
        Http.Request
            .get("http://httpbin.org/status/404")
            .then(function(value) {
                verify(false, "this should not be fulfilled!")
            }, function(reason) {
                compare(reason.status, 404);
                done();
            });
        async.wait(timeout);
    }

    function test_then_fulfilled() {
        var promise = Http.Request
            .get("http://httpbin.org/get")
            .then(function(resp){return resp;});

        sleep(timeout);

        promise.then(function(value) {
           compare(value.status, 200);
           done();
        });

        async.wait(timeout);
    }

    function test_then_multiple() {
        var req = Http.Request
            .get("http://httpbin.org/get")

        var count = 0;
        req.then(function() {
            count++;
        });
        req.then(function() {
            count++;
        });
        req.then(function() {
            count++;
            done();
        });

        async.wait(timeout);

        compare(count, 3);
    }

    function test_then_chaining_1() {
        var p1 = Http.Request
            .get("http://httpbin.org/get?req=1")
            .then(function(value) {
                var req = parseInt(value.body.args.req)+1;
                return Http.Request
                    .get("http://httpbin.org/get?req=" + req)
                    .then(function(res) {
                        return res;
                    });
            });

        var p2 = p1.then(function(value2) {
                compare(value2.body.args.req, "2");
                done();
                return 1337;
            }, function(reason) {
                verify(false, reason);
                done();
            });

        async.wait(timeout);
    }

    function test_then_chaining_2() {
        var p1 = Http.Request
            .get("http://httpbin.org/get?req=1")
            .then();

        var p2 = p1.then(function(value) {
                compare(value.body.args.req, "1");
                done();
            });

        async.wait(timeout);
    }

    function test_then_chaining_3() {
        var p1 = Http.Request
        .get("http://httpbin.org/get")
        .then("42");

        var p2 = p1.then(function(value) {
            compare(value, "42");
            done();
        });

        async.wait(timeout);
    }

    function test_catch() {
        var errStr = "Gotta catch em all!";
        Http.Request
            .get("http://httpbin.org/get")
            .then(function(value) {
                throw new Error(errStr);
            })
            .catch(function(err) {
                compare(err.message, errStr);
                done();
            });

        async.wait(timeout);
    }

    function test_ssl() {
        var fingerprint = "";
        Http.Request
            .get("https://api.github.com")
            .on("secureConnect", function(event) {
                var cert = event.getPeerCertificate();
                fingerprint = cert.fingerprint;
            })
            .end(function() {
                done();
            })

        async.wait(timeout);

        // This fingerprint might need to be changed when the cert is renewed
        compare(fingerprint, "5f:f1:60:31:09:04:3e:f2:90:d2:b0:8a:50:38:04:e8:37:9f:bc:76");
    }

    function test_responseType_auto() {
        Http.Request
            .get("http://httpbin.org/get")
            .responseType(Http.ResponseType.Auto)
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(res.body.url, "http://httpbin.org/get")
                done();
            });

        async.wait(timeout);
    }

    function test_responseType_json() {
        Http.Request
            .get("http://httpbin.org/get")
            .responseType(Http.ResponseType.Json)
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(res.body.url, "http://httpbin.org/get")
                done();
            });

        async.wait(timeout);
    }

    function test_responseType_not_json() {
        Http.Request
            .get("http://httpbin.org/html")
            .responseType(Http.ResponseType.Json)
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(typeof(Object()), typeof(res.body))
                done();
            });

        async.wait(timeout);
    }

    function test_responseType_text() {
        Http.Request
            .get("http://httpbin.org/html")
            .responseType(Http.ResponseType.Text)
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(typeof(String()), typeof(res.body))
                done();
            });

        async.wait(timeout);
    }

    function test_responseType_arraybuffer() {
        Http.Request
            .get("http://httpbin.org/image/jpeg")
            .responseType(Http.ResponseType.ArrayBuffer)
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                compare(res.body.byteLength, 35588);
                compare(typeof(new ArrayBuffer(0)), typeof(res.body));
                done();
            });

        async.wait(timeout);
    }
}
