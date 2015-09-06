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
    
    if ( self != nil )
    {
        FMRArrayInputPort* pArrayInputPort =
            [[FMRArrayInputPort alloc] initWithName:@"Board State Array"
                                          uniqueKey:@"Form.boardState"];
        
        [self addPort:pArrayInputPort portGroup:@"DAFLightsOutSolverNode"];
        
        FMRPrimitiveInputPort* pPrimitiveInputPort =
            [[FMRPrimitiveInputPort alloc] initWithName:@"Solve Board Pulse"
                                              uniqueKey:@"Form.solveBoardPulse"
                                           defaultValue:[PMRPrimitive primitiveWithBooleanValue:NO]];
        
        [self addPort:pPrimitiveInputPort portGroup:@"DAFLightsOutSolverNode"];
        
        FMRPrimitiveOutputPort* pPrimitiveOutputPort =
            [[FMRPrimitiveOutputPort alloc] initWithName:@"Solving Board"
                                               uniqueKey:@"Form.solvingBoard"];
        
        [self addPort:pPrimitiveOutputPort portGroup:@"DAFLightsOutSolverNode"];
        
        FMRArrayOutputPort* pArrayOutputPort =
            [[FMRArrayOutputPort alloc] initWithName:@"Board State Toggle Pulse"
                                           uniqueKey:@"Form.boardStateTogglePulse"];
        
        [self addPort:pArrayOutputPort portGroup:@"DAFLightsOutSolverNode"];
    }
    
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
