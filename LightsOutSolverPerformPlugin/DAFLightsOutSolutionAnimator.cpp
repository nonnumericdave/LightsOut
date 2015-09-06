//
//  DAFLightsOutSolutionAnimator.cpp
//  LightsOut
//
//  Created by David Flores on 5/11/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#include "DAFLightsOutSolutionAnimator.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DAF::LightsOutSolutionAnimator::LightsOutSolutionAnimator(ILightsOutSolutionAnimatorSink* pLightsOutSolutionAnimatorSink,
                                                          double rToggleSelectionFrameDeltaSeconds,
                                                          double rToggleDominoFrameDeltaSeconds,
                                                          const std::vector<bool>& kvbOptimalSolutionMatrix) :
    _kpLightsOutSolutionAnimatorSink(pLightsOutSolutionAnimatorSink),
    _krToggleSelectionFrameDeltaSeconds(rToggleSelectionFrameDeltaSeconds),
    _krToggleDominoFrameDeltaSeconds(rToggleDominoFrameDeltaSeconds),
    _kvbOptimalSolutionMatrix(kvbOptimalSolutionMatrix),
    _rNextToggleFrameDeltaSeconds(0.0),
    _bAnimationHasEnded(false),
    _bConsumerNeedsNextFrame(true)
{
    _kvuToggleElementIndices.reserve(0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DAF::LightsOutSolutionAnimator::~LightsOutSolutionAnimator()
{
    if ( _threadAnimator.joinable() )
    {
        _threadAnimator.join();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
DAF::LightsOutSolutionAnimator::StartAnimation()
{
    if ( _threadAnimator.joinable() )
        return;
    
    _kpLightsOutSolutionAnimatorSink->AnimationHasStarted();

    _threadAnimator = std::thread(&DAF::LightsOutSolutionAnimator::AnimationThread, this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
DAF::LightsOutSolutionAnimator::UpdateFrameDelta(double rFrameDeltaSeconds)
{
    std::unique_lock<std::mutex> lock(_mutexAnimator);
    
    if ( _bAnimationHasEnded )
    {
        _kpLightsOutSolutionAnimatorSink->AnimationHasEnded();
        return;
    }
    
    _rNextToggleFrameDeltaSeconds -= rFrameDeltaSeconds;
    if ( _bConsumerNeedsNextFrame || _rNextToggleFrameDeltaSeconds > 0.0 )
        return;
    
    _kpLightsOutSolutionAnimatorSink->ToggleStateOfElements(_kvuToggleElementIndices);
    _kvuToggleElementIndices.clear();
    
    _bConsumerNeedsNextFrame = true;
    lock.unlock();
    _conditionVariableAnimator.notify_all();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
DAF::LightsOutSolutionAnimator::AnimationThread()
{
    std::unique_lock<std::mutex> lock(_mutexAnimator);
    
    const std::size_t kuBoardElements = _kvbOptimalSolutionMatrix.size();
    const double krBoardDimension = std::sqrt(kuBoardElements);
    const std::size_t kuBoardDimension = static_cast<std::size_t>(krBoardDimension);
    
    for (std::size_t uElementIndex = 0; uElementIndex < kuBoardElements; ++uElementIndex)
    {
        if ( ! _kvbOptimalSolutionMatrix[uElementIndex] )
            continue;
        
        // Toggle selected element
        _conditionVariableAnimator.wait(lock, [this]() -> bool { return _bConsumerNeedsNextFrame; });
        
        _kvuToggleElementIndices.clear();
        _kvuToggleElementIndices.push_back(uElementIndex);
        _rNextToggleFrameDeltaSeconds = _krToggleDominoFrameDeltaSeconds;
        _bConsumerNeedsNextFrame = false;

        // Toggle additional elements in row and column
        const std::size_t kuElementRowIndex = uElementIndex / kuBoardDimension;
        const std::size_t kuElementColumnIndex = uElementIndex % kuBoardDimension;
        
        std::size_t uTopElementRowIndex = kuElementRowIndex;
        std::size_t uBottomElementRowIndex = kuElementRowIndex;
        std::size_t uLeftElementColumnIndex = kuElementColumnIndex;
        std::size_t uRightElementColumnIndex = kuElementColumnIndex;
        for (;;)
        {
            _conditionVariableAnimator.wait(lock, [this]() -> bool { return _bConsumerNeedsNextFrame; });
            
            _kvuToggleElementIndices.clear();
            
            if ( uTopElementRowIndex > 0 )
            {
                --uTopElementRowIndex;
                
                const std::size_t kuTopElementIndex =
                    uTopElementRowIndex * kuBoardDimension +
                    kuElementColumnIndex;
                
                _kvuToggleElementIndices.push_back(kuTopElementIndex);
            }
            
            if ( uBottomElementRowIndex < (kuBoardDimension - 1) )
            {
                ++uBottomElementRowIndex;
                
                const std::size_t kuBottomElementIndex =
                    uBottomElementRowIndex * kuBoardDimension +
                    kuElementColumnIndex;
                
                _kvuToggleElementIndices.push_back(kuBottomElementIndex);
            }
            
            if ( uLeftElementColumnIndex > 0 )
            {
                --uLeftElementColumnIndex;
                
                const std::size_t kuLeftElementIndex =
                    kuElementRowIndex * kuBoardDimension +
                    uLeftElementColumnIndex;
                
                _kvuToggleElementIndices.push_back(kuLeftElementIndex);
            }
            
            if ( uRightElementColumnIndex < (kuBoardDimension - 1) )
            {
                ++uRightElementColumnIndex;
                
                const std::size_t kuRightElementIndex =
                    kuElementRowIndex * kuBoardDimension +
                    uRightElementColumnIndex;
                
                _kvuToggleElementIndices.push_back(kuRightElementIndex);
            }
            
            _rNextToggleFrameDeltaSeconds = _krToggleDominoFrameDeltaSeconds;
            _bConsumerNeedsNextFrame = false;
            
            if ( _kvuToggleElementIndices.empty() )
                break;
        }
        
        _rNextToggleFrameDeltaSeconds = _krToggleSelectionFrameDeltaSeconds;
    }
    
    lock.unlock();
    
    std::this_thread::sleep_for(std::chrono::seconds(15));
    
    lock.lock();
    
    _bAnimationHasEnded = true;
}
