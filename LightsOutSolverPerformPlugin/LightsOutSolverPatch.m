//
//  LightsOutSolverPatch.m
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#import "LightsOutSolverPatch.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation LightsOutSolverPatch

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processPatchWithContext:(PMRProcessContext*)context
{
    // Start off with a black color.
    RIColorRGBA randomColor = RIColorBlackRGBA;
    
    // Get the value from the on/off input.
    BOOL onOff = _onOffInput.booleanValue;
    
    // If the input is on, create a random color.
    if (onOff)
        randomColor = RIColorMakeRGBA(randomFloat(),
                                      randomFloat(),
                                      randomFloat(),
                                      1.0);
    
    // Store the color in the color output.
    _colorOutput.colorValue = randomColor;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float randomFloat()
{
    return ((double)arc4random() / 0x100000000);
}

@end
