//
//  DAFLightsOutDetectorNode.h
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef DAFLightsOutDetectorNode_h
#define DAFLightsOutDetectorNode_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutDetectorNode : FMRViewNode

// NSObject
- (instancetype)init;

// FMRNode
+ (NSString*)defaultName;
+ (NSString *)libraryDescription;
+ (NSString *)libraryCategory;
+ (NSString*)processClassName;

@end

#endif
