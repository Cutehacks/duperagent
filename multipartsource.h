// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#ifndef MULTIPARTSOURCE_H
#define MULTIPARTSOURCE_H

#include <QtCore/QObject>
#include <QtCore/QString>

namespace com { namespace cutehacks { namespace duperagent {

class AbstractMultipartSource : public QObject
{
    Q_OBJECT

public:
    explicit AbstractMultipartSource(QObject *parent = Q_NULLPTR);

    virtual ~AbstractMultipartSource();

    virtual QByteArray data() const = 0;
    virtual QString mimeType() const = 0;
};

} } }

#endif // MULTIPARTSOURCE_H
