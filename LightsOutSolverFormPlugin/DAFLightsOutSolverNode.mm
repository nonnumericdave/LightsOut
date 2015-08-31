//
//  DAFLightsOutSolverNode.mm
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#include "DAFLightsOutSolverNode.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation DAFLightsOutSolverNode

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (instancetype)init
{
    self = [super init];
    
    return self;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (NSString*)defaultName
{
    return @"Lights Out Solver";
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
    return @"DAFLightsOutSolverPatch";
}

@end
