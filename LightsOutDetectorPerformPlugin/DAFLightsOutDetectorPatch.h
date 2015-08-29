//
//  DAFLightsOutDetectorPatch.h
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef DAFLightsOutDetectorPatch_h
#define DAFLightsOutDetectorPatch_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutDetectorPatch : PMRViewPatch

// NSObject
- (instancetype)init;
- (void)dealloc;

// PMRViewPatch
+ (Class)viewClass;

@property (nonatomic, readonly) RIImageView* view;

@end

#endif
