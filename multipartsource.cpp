// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#include "multipartsource.h"

namespace com { namespace cutehacks { namespace duperagent {

AbstractMultipartSource::AbstractMultipartSource(QObject *parent) :
    QObject(parent)
{ }

AbstractMultipartSource::~AbstractMultipartSource()
{ }

} } }
