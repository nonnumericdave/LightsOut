//
//  DAFLightsOutRecognizerPatch.mm
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#include "DAFLightsOutRecognizerPatch.h"
#include "DAFLightsOutRecognizer.h"
#include "DAFOpenCVCamera.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static const void* g_pkvKeyValueObservingContext = nullptr;
static const dispatch_semaphore_t g_dispatchSemaphoreImageProcessing =
    ::dispatch_semaphore_create(3);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutRecognizerPatch ()

// NSKeyValueObserving
- (void)observeValueForKeyPath:(NSString*)pKeyPathString
                      ofObject:(id)pObject
                        change:(NSDictionary*)pChangeDictionary
                       context:(void*)pvContext;

// DAFLightsOutRecognizerPatch
- (void)setBoardStateArray:(NSArray*)pBoardStateArray;

@end

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
void
DispatchBoardStateResult(DAFLightsOutRecognizerPatch* pLightsOutRecognizerPatch,
                         const std::vector<bool>& kvbStateMatrix,
                         const std::size_t kuDimension)
{
    const NSUInteger kuBoardElements = kvbStateMatrix.size();
    NSMutableArray* pBoardStateArray =
        [NSMutableArray arrayWithCapacity:kuBoardElements];
    
    for (bool bElementState : kvbStateMatrix)
    {
        PMRPrimitive* pElementStatePrimitive =
            [PMRPrimitive primitiveWithBooleanValue:bElementState];
        
        [pBoardStateArray addObject:pElementStatePrimitive];
    }
    
    dispatch_block_t dispatchBlock =
        ^void(void)
        {
            [pLightsOutRecognizerPatch setBoardStateArray:pBoardStateArray];
        };
    
    ::dispatch_async(::dispatch_get_main_queue(), dispatchBlock);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation DAFLightsOutRecognizerPatch
{
    DAFOpenCVCamera* _pOpenCVCamera;
}

@dynamic view;

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
+ (Class)viewClass
{
    return [RIImageView class];
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
        
        bool bCanProcessImage =
            ::dispatch_semaphore_wait(::g_dispatchSemaphoreImageProcessing, 0) == 0;
        
        if ( bCanProcessImage )
        {
            const cv::Mat kmatImage = _pOpenCVCamera.matrix;
            
            dispatch_block_t dispatchBlock =
                ^void(void)
                {
                    std::vector<bool> vbStateMatrix;
                    std::size_t uDimension;
                    
                    bool bDidRecognizeBoardState =
                        DAF::RecognizeLightsOutBoardStateFromImage(kmatImage,
                                                                   vbStateMatrix,
                                                                   uDimension);
                    
                    if ( bDidRecognizeBoardState )
                        ::DispatchBoardStateResult(self, vbStateMatrix, uDimension);
                    
                    ::dispatch_semaphore_signal(::g_dispatchSemaphoreImageProcessing);
                };
            
            ::dispatch_async(::dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0),
                             dispatchBlock);
        }
    }
    else
    {
        [super observeValueForKeyPath:pKeyPathString
                             ofObject:pObject
                               change:pChangeDictionary
                              context:pvContext];
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)setBoardStateArray:(NSArray*)pBoardStateArray
{
    _boardState.value = pBoardStateArray;
    
    [self setShouldProcessNextFrame];
}

@end
