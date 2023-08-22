/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "StringUtil.hpp"

#import <Foundation/Foundation.h>

namespace util
{

std::string convertNSStringToStdString(NSString* nsString) {
    if (!nsString) {
        return std::string();
    }

    const char* cString = [nsString UTF8String];
    if (!cString) {
        return std::string();
    }

    return std::string(cString);
}

} //util
