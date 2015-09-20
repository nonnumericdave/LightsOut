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
const double g_krToggleSelectionFrameDeltaSeconds = 1;
const double g_krToggleDominoFrameDeltaSeconds = 0.5;
const double g_krAnimationDoneSleepSeconds = 5;
NSString* const g_pLayerAnimationKeyString = @"DAFLightsOutBoardViewLayerAnimationKey";

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutBoardView ()

// DAFLightsOutBoardView
- (nonnull NSArray<NSNumber*>*)initialBoardState;
- (void)setInitialBoardState:(nonnull NSArray<NSNumber*>*)pInitialBoardStateArray;
- (BOOL)isSolving;
- (void)loadInitialBoardGrid:(nonnull NSArray<NSNumber*>*)pInitialBoardStateArray;
- (void)loadInitialBoardGridState;
- (void)processSolveFromInitialBoardState;
- (void)startAnimation;
- (void)stopAnimation;
- (void)updateForDisplayLink:(CADisplayLink*)pDisplayLink;
- (void)toggleStateForElementAtIndex:(NSUInteger)uElementIndex;

@end

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LightsOutSolutionAnimatorSink : public DAF::ILightsOutSolutionAnimatorSink
{
public:
    // LightsOutSolutionAnimatorSink
    LightsOutSolutionAnimatorSink(DAFLightsOutBoardView* pLightsOutBoardView);
    
private:
    // ILightsOutSolutionAnimatorSink
    virtual void AnimationHasStarted() override;
    virtual void AnimationHasEnded() override;
    virtual void ToggleStateOfElements(const std::vector<std::size_t>& kvuToggleElementIndices) override;
    
    DAFLightsOutBoardView* _pLightsOutBoardView;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LightsOutSolutionAnimatorSink::LightsOutSolutionAnimatorSink(DAFLightsOutBoardView* pLightsOutBoardView) :
    _pLightsOutBoardView(pLightsOutBoardView)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
LightsOutSolutionAnimatorSink::AnimationHasStarted()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
LightsOutSolutionAnimatorSink::AnimationHasEnded()
{
    [_pLightsOutBoardView stopAnimation];
    [_pLightsOutBoardView loadInitialBoardGridState];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
LightsOutSolutionAnimatorSink::ToggleStateOfElements(const std::vector<std::size_t>& kvuToggleElementIndices)
{
    for (const std::size_t kuElementIndex : kvuToggleElementIndices)
        [_pLightsOutBoardView toggleStateForElementAtIndex:kuElementIndex];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation DAFLightsOutBoardView
{
    NSArray<NSNumber*>* _pInitialBoardStateArray;
    BOOL _boolIsSolving;
    
    std::vector<CALayer*> _vBoardElementLayers;

    CADisplayLink* _pDisplayLink;
    LightsOutSolutionAnimatorSink* _pLightsOutSolutionAnimatorSink;
    DAF::LightsOutSolutionAnimator* _pLightsOutSolutionAnimator;
    
    std::mutex _mutex;
}

@dynamic initialBoardState;
@dynamic isSolving;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)dealloc
{
    [self stopAnimation];
    
    delete _pLightsOutSolutionAnimatorSink;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (instancetype)initWithFrame:(CGRect)rectFrame
{
    self = [super initWithFrame:rectFrame];
    
    if ( self != nil )
    {
        _pInitialBoardStateArray = [NSArray array];
        _boolIsSolving = NO;

        _pLightsOutSolutionAnimatorSink = new LightsOutSolutionAnimatorSink(self);
    }
    
    return self;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)layoutSubviews
{
    [super layoutSubviews];
    
    std::unique_lock<std::mutex> uniqueLock(_mutex);
    
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
- (void)solveFromInitialBoardState
{
    [self performSelectorOnMainThread:@selector(processSolveFromInitialBoardState)
                           withObject:nil
                        waitUntilDone:NO];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (nonnull NSArray<NSNumber*>*)initialBoardState
{
    _mutex.lock();
    
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
        
    uniqueLock.unlock();
    
    [self performSelectorOnMainThread:@selector(loadInitialBoardGrid:)
                          withObject:[pInitialBoardStateArray copy]
                       waitUntilDone:NO];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (BOOL)isSolving
{
    return _boolIsSolving;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)loadInitialBoardGrid:(nonnull NSArray<NSNumber*>*)pInitialBoardStateArray
{
    // Called on main thread.
    
    _mutex.lock();
    
    _pInitialBoardStateArray = [pInitialBoardStateArray copy];
    
    const std::size_t kuBoardElementCount =
        _vBoardElementLayers.size();

    const NSUInteger kuUpdatedBoardElementCount =
        [_pInitialBoardStateArray count];
    
    if ( kuBoardElementCount == kuUpdatedBoardElementCount )
    {
        _mutex.unlock();
        
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
    
    _mutex.unlock();
    
    [self loadInitialBoardGridState];
    
    [self setNeedsLayout];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)loadInitialBoardGridState
{
    // Called on main thread.
    
    std::unique_lock<std::mutex> uniqueLock(_mutex);
    
    assert( [_pInitialBoardStateArray count] == _vBoardElementLayers.size() );
    
    [_pInitialBoardStateArray enumerateObjectsUsingBlock:
        ^(NSNumber* _Nonnull pElementStateNumber, NSUInteger uElementIndex, BOOL* _Nonnull pboolStopEnumerating)
        {
            CALayer* pLayer = _vBoardElementLayers[uElementIndex];
            
            [pLayer removeAnimationForKey:(::g_pLayerAnimationKeyString)];
            
            pLayer.opacity =
                [pElementStateNumber boolValue] ?
                    1.0 :
                    0.0;
        }];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processSolveFromInitialBoardState
{
    // Called on main thread.
    
    [self stopAnimation];
    [self loadInitialBoardGridState];
    [self startAnimation];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)startAnimation
{
    // Called on main thread.

    std::unique_lock<std::mutex> uniqueLock(_mutex);
    
    const std::size_t kuBoardElementCount = [_pInitialBoardStateArray count];
    const std::size_t kuBoardDimension = static_cast<std::size_t>(std::sqrt(kuBoardElementCount));
    
    std::vector<bool> vbStateMatrix;
    vbStateMatrix.reserve(kuBoardElementCount);
    for (NSNumber* pElementStateNumber in _pInitialBoardStateArray)
        vbStateMatrix.push_back([pElementStateNumber boolValue]);
    
    DAF::LightsOutSolver lightsOutSolver(kuBoardDimension);
    std::vector<bool> vbOptimalSolutionMatrix;
    if ( ! lightsOutSolver.Solve(vbStateMatrix, vbOptimalSolutionMatrix) )
        return;
    
    assert( _pLightsOutSolutionAnimator == nullptr );
    
    _pLightsOutSolutionAnimator =
        new DAF::LightsOutSolutionAnimator(_pLightsOutSolutionAnimatorSink,
                                           ::g_krToggleSelectionFrameDeltaSeconds,
                                           ::g_krToggleDominoFrameDeltaSeconds,
                                           ::g_krAnimationDoneSleepSeconds,
                                           vbOptimalSolutionMatrix);
    
    assert( _pDisplayLink == nil );
    
    _pDisplayLink =
        [CADisplayLink displayLinkWithTarget:self selector:@selector(updateForDisplayLink:)];
    
    [_pDisplayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
    
    _pLightsOutSolutionAnimator->StartAnimation();
    
    uniqueLock.unlock();
    
    [self willChangeValueForKey:@"isSolving"];
    
    uniqueLock.lock();
    
    _boolIsSolving = YES;
    
    uniqueLock.unlock();
    
    [self didChangeValueForKey:@"isSolving"];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)stopAnimation
{
    _mutex.lock();
    
    if ( _pLightsOutSolutionAnimator != nullptr )
    {
        _pLightsOutSolutionAnimator->StopAnimation();
        delete _pLightsOutSolutionAnimator;
        _pLightsOutSolutionAnimator = nullptr;
    }
    
    [_pDisplayLink invalidate];

    _pDisplayLink = nil;
    
    _mutex.unlock();
    
    [self willChangeValueForKey:@"isSolving"];
    
    _mutex.lock();
    
    _boolIsSolving = NO;
    
    _mutex.unlock();
    
    [self didChangeValueForKey:@"isSolving"];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)updateForDisplayLink:(CADisplayLink*)pDisplayLink
{
    // Called on main thread.
    
    assert( pDisplayLink == _pDisplayLink );
    
    assert( _pLightsOutSolutionAnimator != nullptr );
    
    _pLightsOutSolutionAnimator->UpdateFrameDelta(pDisplayLink.duration);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)toggleStateForElementAtIndex:(NSUInteger)uElementIndex
{
    // Called on main thread.
    
    assert( uElementIndex < _vBoardElementLayers.size() );
    
    CALayer* pLayer = _vBoardElementLayers[uElementIndex];
    
    CAAnimation* pPreviousAnimation =
        [pLayer animationForKey:(::g_pLayerAnimationKeyString)];
    
    CGFloat rOpacityFromValue;
    CGFloat rOpacityToValue;
    if ( pPreviousAnimation != nil )
    {
        CALayer* pPresentationLayer = pLayer.presentationLayer;
        rOpacityFromValue = pPresentationLayer.opacity;
        
        assert( [pPreviousAnimation isKindOfClass:[CABasicAnimation class]] );
        
        CABasicAnimation* pPreviousBasicAnimation =
            static_cast<CABasicAnimation*>(pPreviousAnimation);
        
        NSNumber* pPreviousToValueNumber = pPreviousBasicAnimation.toValue;
        rOpacityToValue =
            ([pPreviousToValueNumber floatValue] > 0.5) ?
                0.0 :
                1.0;
        
        [pLayer removeAnimationForKey:(::g_pLayerAnimationKeyString)];
    }
    else
    {
        rOpacityFromValue = pLayer.opacity;
        rOpacityToValue = (rOpacityFromValue > 0.5) ? 0.0 : 1.0;
    }
    
    pLayer.opacity = rOpacityToValue;
    
    CABasicAnimation* pBasicAnimation =
        [CABasicAnimation animationWithKeyPath:@"opacity"];
    
    pBasicAnimation.fromValue = [NSNumber numberWithFloat:rOpacityFromValue];
    pBasicAnimation.toValue = [NSNumber numberWithFloat:rOpacityToValue];
    
    CFTimeInterval timeIntervalAnimation =
        std::abs(rOpacityFromValue - rOpacityToValue) * ::g_krToggleSelectionFrameDeltaSeconds;
    
    pBasicAnimation.duration = timeIntervalAnimation;
    
    pBasicAnimation.timingFunction =
        [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];
    
    [pLayer addAnimation:pBasicAnimation forKey:(::g_pLayerAnimationKeyString)];
}

@end
