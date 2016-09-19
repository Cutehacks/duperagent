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
        Http.request.config({
            proxy: "system" // for debugging with Charles Proxy
        });
        Http.request.clearCookies();
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
        Http.request
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
        Http.request
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

    function test_post_form() {
        Http.request
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
        Http.request
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
        Http.request
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
        Http.request
            .del("http://httpbin.org/delete")
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                done();
            });

        async.wait(timeout);
    }

    function test_gzip() {
        Http.request
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
        Http.request
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
        Http.request
            .get("http://httpbin.org/status/404")
            .end(function(err, res){
                verify(err);
                compare(res.status, 404);
                done();
            });

        async.wait(timeout);
    }

    function test_redirect_5() {
        Http.request
            .get("http://httpbin.org/redirect/5")
            .end(function(err, res){
                verify(!err, err);
                compare(res.status, 200);
                done();
            });

        async.wait(timeout);
    }

    function test_redirect_7_fail() {
        Http.request
        .get("http://httpbin.org/redirect/7")
        .end(function(err, res){
            verify(err);
            compare(res.status, 302);
            done();
        });

        async.wait(timeout);
    }

    function test_redirect_7_pass() {
        Http.request
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
        Http.request
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
        Http.request
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
        Http.request
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
        Http.request.cookie = "food=pizza; domain=httpbin.org";

        Http.request
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
        Http.request.cookie = "persistent=for1year; domain=httpbin.org; expires=" + expiresNextYear();

        Http.request
            .get("http://httpbin.org/cookies/set")
            .query({foo: "bar"})
            .end(function(err, res){
                verify(res.body.cookies.persistent)
                verify(Http.request.cookie !== "");
                done();
            });

        async.wait(timeout);
    }

    function test_cookies_delete() {
        Http.request.cookie = "victim=me; domain=httpbin.org; path=/; expires=" + expiresNextYear();

        Http.request
            .get("http://httpbin.org/cookies/delete?victim=")
            .end(function(err, res){
                verify(!res.body.cookies.victim)
                verify(!Http.request.cookie || Http.request.cookie.indexOf("victim") < 0);
                done();
            });

        async.wait(timeout);
    }

    function test_cache() {
        Http.request
            .get("http://httpbin.org/cache/3")
            .end(function(err, res){
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
        async.clear();

        Http.request
            .get("http://httpbin.org/cache/3")
            .end(function(err, res){
                verify(res.fromCache);
                done();
            });

        async.wait(timeout);
        async.clear();

        sleep(5000);

        Http.request
            .get("http://httpbin.org/cache/3")
            .end(function(err, res){
                verify(!res.fromCache);
                done();
            });

        async.wait(timeout);
    }

    function test_multipart() {
        Http.request
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

        Http.request
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

        Http.request
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
        Http.request
            .get("http://httpbin.org/get")
            .then(function(resp) {
                done();
                return resp;
            });
        async.wait(timeout);
    }

    function test_then_rejected() {
        Http.request
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
        var promise = Http.request
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
        var req = Http.request
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
        var p1 = Http.request
            .get("http://httpbin.org/get?req=1")
            .then(function(value) {
                var req = parseInt(value.body.args.req)+1;
                return Http.request
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

    function test_ssl() {
        var fingerprint = "";
        Http.request
            .get("https://httpbin.org/get")
            .on("secureConnect", function(event) {
                var cert = event.getPeerCertificate();
                fingerprint = cert.fingerprint;
            })
            .end(function() {
                done();
            })

        async.wait(timeout);

        // This fingerprint might need to be changed when the cert is renewed
        compare(fingerprint, "9d:01:5c:8e:fd:4d:df:71:a4:99:ce:29:93:40:3f:5f:ee:74:d0:95");
    }

}
