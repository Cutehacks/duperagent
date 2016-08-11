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
}
