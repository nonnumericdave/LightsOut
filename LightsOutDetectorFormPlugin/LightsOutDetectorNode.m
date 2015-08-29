//
//  LightsOutDetectorNode.m
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#import "LightsOutDetectorNode.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation LightsOutDetectorNode

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (NSString*)defaultName
{
    return @"Lights Out Detector";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (NSString*)processClassName
{
    return @"LightsOutDetectorPatch";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (instancetype)init
{
    if ((self = [super init]) != nil)
	{
        // Inputs
        [self addPort:[[FMRPrimitiveInputPort alloc] initWithName:@"On / Off" uniqueKey:@"Form.onOffInput" defaultValue:[PMRPrimitive primitiveWithBooleanValue:NO]] portGroup:nil];
        
        // Outputs
        [self addPort:[[FMRPrimitiveOutputPort alloc] initWithName:@"Color" uniqueKey:@"Form.colorOutput"] portGroup:nil];
    }
	
    return self;
}

@end
