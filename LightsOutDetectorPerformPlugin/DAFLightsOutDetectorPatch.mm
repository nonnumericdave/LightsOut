//
//  DAFLightsOutDetectorPatch.mm
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#include "DAFLightsOutDetectorPatch.h"
#include "DAFOpenCVCamera.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const void* g_pkvKeyValueObservingContext = nullptr;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutDetectorPatch ()

// NSObject
- (instancetype)init;
- (void)dealloc;

// NSKeyValueObserving
- (void)observeValueForKeyPath:(NSString*)pKeyPathString
					  ofObject:(id)pObject
						change:(NSDictionary*)pChangeDictionary
					   context:(void*)pvContext;

@end

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation DAFLightsOutDetectorPatch
{
	DAFOpenCVCamera* _pOpenCVCamera;
}

@dynamic view;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (Class)viewClass
{
	return [RIImageView class];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (instancetype)init
{
	self = [super init];
	
	if ( self != nil )
	{
		_pOpenCVCamera = [DAFOpenCVCamera sharedOpenCVCamera];
		
		[_pOpenCVCamera addObserver:self
						 forKeyPath:@"image"
							options:0
							context:&::g_pkvKeyValueObservingContext];
	}
	
	return self;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)dealloc
{
	[_pOpenCVCamera removeObserver:self
						forKeyPath:@"image"
						   context:&::g_pkvKeyValueObservingContext];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)observeValueForKeyPath:(NSString*)pKeyPathString
					  ofObject:(id)pObject
						change:(NSDictionary*)pChangeDictionary
					   context:(void*)pvContext
{
	if ( pvContext == &::g_pkvKeyValueObservingContext )
	{
		assert( [pKeyPathString isEqualToString:@"image"] );
		assert( pObject == _pOpenCVCamera );
		
		self.view.image = _pOpenCVCamera.image;
	}
	else
	{
		[super observeValueForKeyPath:pKeyPathString
							 ofObject:pObject
							   change:pChangeDictionary
							  context:pvContext];
	}
}

@end
