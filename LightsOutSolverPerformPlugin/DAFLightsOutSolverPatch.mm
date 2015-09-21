//
//  DAFLightsOutSolverPatch.mm
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#include "DAFLightsOutSolverPatch.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutSolverPatch ()

// DAFLightsOutSolverPatch
- (void)setSolvingBoardState:(BOOL)boolSolvingBoard;
- (void)processSolution:(const std::vector<bool>&)kvbOptimalSolutionMatrix;
- (void)processNoSolution;
- (void)processSolutionCompletion;

@end

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LightsOutSolutionAnimatorSink : public DAF::ILightsOutSolutionAnimatorSink
{
public:
    // LightsOutSolutionAnimatorSink
    LightsOutSolutionAnimatorSink(DAFLightsOutSolverPatch* pLightsOutSolverPatch);
    
    void ClearStateOfElements();
    
private:
    // ILightsOutSolutionAnimatorSink
    virtual void AnimationHasStarted() override;
    virtual void AnimationHasEnded() override;
    virtual void ToggleStateOfElements(const std::vector<std::size_t>& kvuToggleElementIndices) override;
    
    DAFLightsOutSolverPatch* _pLightsOutSolverPatch;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LightsOutSolutionAnimatorSink::LightsOutSolutionAnimatorSink(DAFLightsOutSolverPatch* pLightsOutSolverPatch) :
    _pLightsOutSolverPatch(pLightsOutSolverPatch)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
LightsOutSolutionAnimatorSink::ClearStateOfElements()
{
    NSArray* pBoardStateTogglePulseArray = _pLightsOutSolverPatch.boardStateTogglePulse.value;
    
    for (PMRPrimitive* pElementTogglePulsePrimitive in pBoardStateTogglePulseArray)
        [pElementTogglePulsePrimitive setBooleanValue:NO];
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
    [_pLightsOutSolverPatch processSolutionCompletion];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
LightsOutSolutionAnimatorSink::ToggleStateOfElements(const std::vector<std::size_t>& kvuToggleElementIndices)
{
    ClearStateOfElements();
    
    NSArray* pBoardStateTogglePulseArray = _pLightsOutSolverPatch.boardStateTogglePulse.value;
    
    for (const std::size_t kuElementIndex : kvuToggleElementIndices)
    {
        PMRPrimitive* pElementTogglePulsePrimitive =
            pBoardStateTogglePulseArray[kuElementIndex];
        
        [pElementTogglePulsePrimitive setBooleanValue:YES];
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation DAFLightsOutSolverPatch
{
    BOOL _boolDidUpdateLastFrame;
    
    LightsOutSolutionAnimatorSink* _pLightsOutSolutionAnimatorSink;
    DAF::LightsOutSolutionAnimator* _pLightsOutSolutionAnimator;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processPatchWithContext:(PMRProcessContext*)pProcessContext
{
    [super processPatchWithContext:pProcessContext];
    
    if ( [_solvingBoard.value booleanValue] || ! [self.solveBoardPulse booleanValue] )
        return;
    
    NSArray* pBoardStateInputArray = self.boardState.value;
    const NSUInteger kuBoardElements = [pBoardStateInputArray count];
    if ( kuBoardElements == 0 )
        return;
    
    const double krBoardDimension = std::sqrt(kuBoardElements);
    if ( std::floor(krBoardDimension) != std::ceil(krBoardDimension) )
        return;
    
    [self setSolvingBoardState:YES];

    NSMutableArray* pBoardStateTogglePulseArray =
        [NSMutableArray arrayWithCapacity:kuBoardElements];
    
    for (NSUInteger uIndex = 0; uIndex < kuBoardElements; ++uIndex)
    {
        PMRPrimitive* pElementTogglePulsePrimitive =
            [PMRPrimitive primitiveWithBooleanValue:NO];
        
        [pBoardStateTogglePulseArray addObject:pElementTogglePulsePrimitive];
    }

    _boardStateTogglePulse.value = pBoardStateTogglePulseArray;
    
    dispatch_block_t dispatchBlock =
        ^void(void)
        {
            assert( kuBoardElements <= std::numeric_limits<std::size_t>::max() );
            
            std::vector<bool> vbStateMatrix;
            vbStateMatrix.reserve(kuBoardElements);
            
            for (PMRPrimitive* pPrimitive : pBoardStateInputArray)
                vbStateMatrix.push_back([pPrimitive booleanValue]);
            
            dispatch_block_t dispatchSolutionBlock = nil;
            
            const std::size_t kuBoardDimension = static_cast<std::size_t>(krBoardDimension);
            
            DAF::LightsOutSolver lightsOutSolver(kuBoardDimension);
            std::vector<bool> vbOptimalSolutionMatrix;
            if ( lightsOutSolver.Solve(vbStateMatrix, vbOptimalSolutionMatrix) )
            {
                dispatchSolutionBlock =
                    ^void(void)
                    {
                        [self processSolution:vbOptimalSolutionMatrix];
                    };
            }
            else
            {
                dispatchSolutionBlock =
                    ^void(void)
                    {
                        [self processNoSolution];
                    };
            }
            
            ::dispatch_async(::dispatch_get_main_queue(), dispatchSolutionBlock);
        };
    
    ::dispatch_async(::dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
                     dispatchBlock);
    
    [self setShouldProcessNextFrame];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processOutput:(PMRPort*)pOutputPort context:(PMRProcessContext*)pProcessContext
{
    [super processOutput:pOutputPort context:pProcessContext];
    
    if ( ! [_solvingBoard.value booleanValue] )
        return;

    if ( _pLightsOutSolutionAnimator != nullptr &&
         pOutputPort == self.boardStateTogglePulse )
    {
        BOOL boolDidUpdateFrame =
            _pLightsOutSolutionAnimator->UpdateFrameDelta(pProcessContext.deltaTime);

        if ( _boolDidUpdateLastFrame && ! boolDidUpdateFrame )
           _pLightsOutSolutionAnimatorSink->ClearStateOfElements();
        
        _boolDidUpdateLastFrame = boolDidUpdateFrame;

        [self setShouldProcessNextFrame];
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)setSolvingBoardState:(BOOL)boolSolvingBoard
{
    [_solvingBoard.value setBooleanValue:boolSolvingBoard];

    [self setShouldProcessNextFrame];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processSolution:(const std::vector<bool>&)kvbOptimalSolutionMatrix
{
    if ( _pLightsOutSolutionAnimatorSink != nullptr ||
         _pLightsOutSolutionAnimator != nullptr )
        return;
    
    _boolDidUpdateLastFrame = NO;
    
    _pLightsOutSolutionAnimatorSink =
        new LightsOutSolutionAnimatorSink(self);
    
    _pLightsOutSolutionAnimator =
        new DAF::LightsOutSolutionAnimator(_pLightsOutSolutionAnimatorSink,
                                           1,
                                           0.25,
                                           5,
                                           false,
                                           kvbOptimalSolutionMatrix);
    
    _pLightsOutSolutionAnimator->StartAnimation();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processNoSolution
{
    if ( _pLightsOutSolutionAnimatorSink != nullptr ||
         _pLightsOutSolutionAnimator != nullptr )
        return;
    
    [self processSolutionCompletion];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processSolutionCompletion
{
    if ( _pLightsOutSolutionAnimator != nullptr )
    {
        delete _pLightsOutSolutionAnimator;
        _pLightsOutSolutionAnimator = nullptr;
    }
 
    if ( _pLightsOutSolutionAnimatorSink != nullptr )
    {
        delete _pLightsOutSolutionAnimatorSink;
        _pLightsOutSolutionAnimatorSink = nullptr;
    }
    
    _boolDidUpdateLastFrame = NO;
    
    [self setSolvingBoardState:NO];
}

@end
