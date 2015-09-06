//
//  DAFLightsOutRecognizerPatch.h
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef DAFLightsOutRecognizerPatch_h
#define DAFLightsOutRecognizerPatch_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutRecognizerPatch : PMRViewPatch

// NSObject
- (instancetype)init;
- (void)dealloc;

// PMRViewPatch
+ (Class)viewClass;

@property (nonatomic, readonly) RIImageView* view;

// DAFLightsOutRecognizerPatch
@property (nonatomic, readonly) PMRArrayOutputPort* boardState;

@end

#endif
