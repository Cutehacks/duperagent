// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

import QtQuick 2.3
import QtTest 1.0

import com.cutehacks.duperagent 1.0 as Http

TestCase {
    id: test2
    name: "imageutils"
    property int timeout: 6000
    signal done

    SignalSpy {
        id: async3
        target: test2
        signalName: "done"
    }

    function initTestCase() {
        Http.Request.config({
            proxy: "system" // for debugging with Charles Proxy
        });
    }

    function test_read_props() {
        var width = 800;
        var height = 600;
        var imageUrl = "https://dummyimage.com/" + width + "x" + height + "/000/fff.jpg";

        Http.Request
            .get(imageUrl)
            .then(function(res) {
                var reader = Http.ImageUtils.createReader(res.body);
                var size = reader.size();
                var fileSize = reader.fileSize();
                verify(fileSize > 0);
                compare(size.width, width);
                compare(size.height, height);
                done();
            });

        async3.wait(timeout);
    }

    function test_scale() {
        var width = 800;
        var height = 600;
        var imageUrl = "https://dummyimage.com/" + width + "x" + height + "/000/fff.jpg";

        // Step 1: Download an image
        var download = Http.Request
            .get(imageUrl)
            .then(function(res) {
                return res.body;
            });

        // Step 2: Scale the image down to 200x200
        var scale = download.then(function(img) {
            var reader = Http.ImageUtils.createReader(img);
            reader.setScaledSize(200, 200, Image.PreserveAspectCrop);
            return reader.read();
        });

        // Step 3: Upload the scaled down image
        var upload = scale.then(function(scaled) {
            Http.Request
                .post("http://httpbin.org/post")
                .attach('jpg', scaled)
                .then(function(res) {
                    compare(res.body.files.jpg, scaled.toJSON())
                    done();
                });
        });

        async3.wait(timeout);
    }
}
