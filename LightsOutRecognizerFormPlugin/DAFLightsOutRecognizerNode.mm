//
//  DAFLightsOutRecognizerNode.mm
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#include "DAFLightsOutRecognizerNode.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation DAFLightsOutRecognizerNode

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (instancetype)init
{
    self = [super init];

    if ( self != nil )
    {
        FMRArrayOutputPort* pArrayOutputPort =
            [[FMRArrayOutputPort alloc] initWithName:@"Board State Array"
                                           uniqueKey:@"Form.boardState"];
        
        [self addPort:pArrayOutputPort portGroup:@"DAFLightsOutRecognizerNode"];
    }
    
    return self;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (NSString*)defaultName
{
    return @"Lights Out Recognizer View";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (NSString*)libraryDescription
{
    return @"";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (NSString*)libraryCategory
{
    return @"Lights Out";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (NSString*)processClassName
{
    return @"DAFLightsOutRecognizerPatch";
}

@end
