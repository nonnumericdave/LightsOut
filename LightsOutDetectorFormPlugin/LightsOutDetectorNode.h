//
//  LightsOutDetectorNode.h
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#import <Former/Former.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface LightsOutDetectorNode : FMRViewNode

// NSObject
- (instancetype)init;

// FMRNode
+ (NSString*)defaultName;
+ (NSString*)processClassName;

@end
