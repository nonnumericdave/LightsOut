//
//  DAFLightsOutRecognizerNode.h
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef DAFLightsOutRecognizerNode_h
#define DAFLightsOutRecognizerNode_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutRecognizerNode : FMRViewNode

// NSObject
- (instancetype)init;

// FMRNode
+ (NSString*)defaultName;
+ (NSString*)libraryDescription;
+ (NSString*)libraryCategory;
+ (NSString*)processClassName;

@end

#endif
