//
//  LightsOutDetectorPatch.h
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#import <Performer/Performer.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface LightsOutDetectorPatch : PMRViewPatch

// PMRPatch
- (void)processPatchWithContext:(PMRProcessContext*)pProcessContext;

// PMRViewPatch
+ (Class)viewClass;

@property (nonatomic, readonly) RIImageView* view;

@end
