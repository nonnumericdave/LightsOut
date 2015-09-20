//
//  DAFLightsOutBoardView.mm
//  LightsOut
//
//  Created by David Flores on 9/19/15.
//  Copyright © 2015 David Flores. All rights reserved.
//

#include "DAFLightsOutBoardView.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const CGFloat g_krLayerSpacing = 20.0;
const CGFloat g_krLayerCornerRadiusPercentageOfSize = 0.25;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutBoardView ()

// UIView
- (void)layoutSubviews;

// DAFLightsOutBoardView
- (nonnull NSArray<NSNumber*>*)initialBoardState;
- (void)setInitialBoardState:(nonnull NSArray<NSNumber*>*)pInitialBoardStateArray;
- (void)loadInitialBoardGrid;
- (void)loadInitialBoardGridState;

@end

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation DAFLightsOutBoardView
{
    NSArray<NSNumber*>* _pInitialBoardStateArray;
    std::vector<CALayer*> _vBoardElementLayers;
    std::mutex _mutex;
}

@dynamic initialBoardState;
@dynamic isSolving;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)layoutSubviews
{
    [super layoutSubviews];
    
    const NSUInteger kuBoardDimension =
        static_cast<NSUInteger>(std::sqrt([_pInitialBoardStateArray count]));
    
    const CGSize ksizeBounds = self.bounds.size;
    const CGFloat krBoundsSize = std::min(ksizeBounds.width, ksizeBounds.height);
    const CGFloat krLayerSize =
        std::floor((krBoundsSize - (kuBoardDimension + 1) * ::g_krLayerSpacing) /
                    kuBoardDimension);
    
    const CGFloat krLayerCornerRadius =
    std::floor(krLayerSize * ::g_krLayerCornerRadiusPercentageOfSize);
    
    CGRect rectLayerFrame =
        ::CGRectMake(::g_krLayerSpacing, ::g_krLayerSpacing, krLayerSize, krLayerSize);

    for (std::size_t uRow = 0; uRow < kuBoardDimension; ++uRow)
    {
        for (std::size_t uColumn = 0; uColumn < kuBoardDimension; ++uColumn)
        {
            CALayer* pLayer = _vBoardElementLayers[uRow * kuBoardDimension + uColumn];
            
            pLayer.frame = rectLayerFrame;
            pLayer.cornerRadius = krLayerCornerRadius;
            
            rectLayerFrame.origin.x += krLayerSize + ::g_krLayerSpacing;
        }
        
        rectLayerFrame.origin.x = ::g_krLayerSpacing;
        rectLayerFrame.origin.y += krLayerSize + ::g_krLayerSpacing;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (nonnull NSArray<NSNumber*>*)initialBoardState
{
    _mutex.lock();
    
    if ( _pInitialBoardStateArray == nil )
        _pInitialBoardStateArray = [NSArray array];
    
    NSArray<NSNumber*>* pInitialBoardStateArray = _pInitialBoardStateArray;
    
    _mutex.unlock();
    
    return pInitialBoardStateArray;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)setInitialBoardState:(nonnull NSArray<NSNumber*>*)pInitialBoardStateArray
{
    std::unique_lock<std::mutex> uniqueLock(_mutex);
    
    if ( [pInitialBoardStateArray isEqual:_pInitialBoardStateArray] )
        return;
    
    const NSUInteger kuUpdatedBoardElementCount =
        [pInitialBoardStateArray count];
    
    const NSUInteger kuUpdatedBoardDimension =
        static_cast<NSUInteger>(std::sqrt([pInitialBoardStateArray count]));
    
    if ( (kuUpdatedBoardDimension * kuUpdatedBoardDimension) != kuUpdatedBoardElementCount )
        [NSException raise:@"BoardStateNotSquareException"
                    format:@"Expecting an NSArray of size N * N, for N-dimension board"];
        
    _pInitialBoardStateArray = [pInitialBoardStateArray copy];
    
    uniqueLock.unlock();
    
   [self performSelectorOnMainThread:@selector(loadInitialBoardGrid)
                          withObject:nil
                       waitUntilDone:NO];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)loadInitialBoardGrid
{
    // Called on main thread.
    
    std::unique_lock<std::mutex> uniqueLock(_mutex);
    
    const std::size_t kuBoardElementCount =
        _vBoardElementLayers.size();

    const NSUInteger kuUpdatedBoardElementCount =
        [_pInitialBoardStateArray count];
    
    if ( kuBoardElementCount == kuUpdatedBoardElementCount )
    {
        [self loadInitialBoardGridState];
        return;
    }
    
    if ( kuBoardElementCount > kuUpdatedBoardElementCount )
    {
        for (std::size_t uElementsToRemoveCount = kuBoardElementCount - kuUpdatedBoardElementCount;
             uElementsToRemoveCount > 0;
             --uElementsToRemoveCount)
        {
            [_vBoardElementLayers.back() removeFromSuperlayer];
            _vBoardElementLayers.pop_back();
        }
    }
    else
    {
        _vBoardElementLayers.reserve(kuUpdatedBoardElementCount);
        
        for (std::size_t uElementsToAddCount = kuUpdatedBoardElementCount - kuBoardElementCount;
             uElementsToAddCount > 0;
             --uElementsToAddCount)
        {
            CALayer* pLayer = [CALayer layer];
            pLayer.backgroundColor = [UIColor whiteColor].CGColor;
            
            _vBoardElementLayers.push_back(pLayer);
            
            [self.layer addSublayer:pLayer];
        }
    }
    
    [self loadInitialBoardGridState];
    
    uniqueLock.unlock();
    
    [self setNeedsLayout];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)loadInitialBoardGridState
{
    // Called on main thread with mutex locked.
    
    assert( [_pInitialBoardStateArray count] == _vBoardElementLayers.size() );
    
    [_pInitialBoardStateArray enumerateObjectsUsingBlock:
        ^(NSNumber* _Nonnull pElementStateNumber, NSUInteger uElementIndex, BOOL* _Nonnull pboolStopEnumerating)
        {
            _vBoardElementLayers[uElementIndex].opacity =
                [pElementStateNumber boolValue] ?
                    1.0 :
                    0.0;
        }];
}

@end
