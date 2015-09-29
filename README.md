DuperAgent is a QML clone of the fantastic [SuperAgent](https://visionmedia.github.io/superagent)
library from VisionMedia.

The library is implemented in Qt/C++ and designed as an alternative to QML's builtin implementation
of XmlHttpRequest.

#Features

* Modern API in comparison to XmlHttpRequest
* Support for multipart/form uploads
* Automatic parsing of response bodies with known content-types

#Limitations

* DuperAgent does not implement the XML API of XmlHttpRequest

#Docs

At the point in time, the API should be almost identical to that of SuperAgent so that documentation
is recommended.

#Usage

First install the library via qpm:

```
qpm install com.cutehacks.duperagent
```

Next register the types in your main.cpp (qpm will automate this in the future):

```
#include "com/cutehacks/duperagent/duperagent.h"
using namespace com::cutehacks;

...

    qmlRegisterSingletonType<duperagent::Request>(
                "com.cutehacks.duperagent",
                1, 0,
                "request",
                duperagent::request_provider);
```

Finally, import the module and start using it inside your .qml files:

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

