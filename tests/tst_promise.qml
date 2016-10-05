// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

import QtQuick 2.3
import QtTest 1.0

import com.cutehacks.duperagent 1.0 as Http

TestCase {
    id: test2
    name: "promise"
    property int timeout: 6000
    signal done

    SignalSpy {
        id: async2
        target: test2
        signalName: "done"
    }

    function test_create() {
        var p1 = Http.promise.create(function(resolve, reject) {
            Http.request
                .get("http://httpbin.org/get")
                .end(function(err, res){
                    resolve(5);
                    done();
                });
        });

        async2.wait(timeout);

        p1.then(function(value) {
            compare(value, 5);
        });
    }

    function test_resolve() {
        var p1 = Http.promise.resolve(5);

        p1.then(function(value) {
            compare(value, 5);
            done();
        });

        async2.wait(timeout);
    }

    function test_reject() {
        var p1 = Http.promise.reject(5);

        p1.then(function(value) {
            verify(false, "Should not get here");
        }, function(reason) {
            compare(reason, 5);
            done();
        });

        async2.wait(timeout);
    }

    function test_all() {
        var p1 = Http.promise.resolve(3);
        var p2 = 1337;
        var p3 = Http.promise.create(function(resolve, reject) {
            Http.request
                .get("http://httpbin.org/get")
                .end(function() {
                    resolve("foo");
                })
        });

        Http.promise.all([p1, p2, p3]).then(function(values) {
            compare(values[0], 3);
            compare(values[1], 1337);
            compare(values[2], "foo");
            done();
        })

        async2.wait(timeout);
    }

    function test_all_fail_fast() {
        var p1 =  Http.promise.reject(3);
        var p2 = 1337;
        var p3 = Http.promise.create(function(resolve, reject) {
            Http.request
            .get("http://httpbin.org/get")
            .end(function() {
                resolve("foo");
            })
        });

        Http.promise.all([p1, p2, p3]).then(function(values) {
            verify(false, "should not get here");
        })
        .catch(function(reason) {
            compare(reason, 3);
            done();
        });

        async2.wait(timeout);
    }

    function test_race() {
        var p1 = Http.request
            .get("http://httpbin.org/delay/1")
            .then(function(resp) {
                return "i won";
            });
        var p2 = Http.request
            .get("http://httpbin.org/delay/3")
            .then(function(resp) {
                return "i lost";
            });

        Http.promise
            .race([p1, p2])
            .then(function(winner) {
                compare(winner, "i won");
                done();
            });

        async2.wait(timeout);
    }
}
