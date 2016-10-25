// Copyright 2016 Cutehacks AS. All rights reserved.
// License can be found in the LICENSE file.

#import <UIKit/UIKit.h>
#import "networkactivityindicator.h"

namespace com { namespace cutehacks { namespace duperagent {

void NetworkActivityIndicator::setEnabledNative(bool enabled)
{
    [[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:(BOOL)enabled];
}

} } }
