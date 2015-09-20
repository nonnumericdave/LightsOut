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
                                                          double rAnimationDoneSleepSeconds,
                                                          const std::vector<bool>& kvbOptimalSolutionMatrix) :
    _kpLightsOutSolutionAnimatorSink(pLightsOutSolutionAnimatorSink),
    _krToggleSelectionFrameDeltaSeconds(rToggleSelectionFrameDeltaSeconds),
    _krToggleDominoFrameDeltaSeconds(rToggleDominoFrameDeltaSeconds),
    _krAnimationDoneSleepSeconds(rAnimationDoneSleepSeconds),
    _kvbOptimalSolutionMatrix(kvbOptimalSolutionMatrix),
    _rNextToggleFrameDeltaSeconds(0.0),
    _bStopAnimationRequest(false),
    _bAnimationHasEnded(false),
    _bConsumerNeedsNextFrame(true)
{
    _kvuToggleElementIndices.reserve(0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DAF::LightsOutSolutionAnimator::~LightsOutSolutionAnimator()
{
    if ( _threadAnimator.joinable() )
        _threadAnimator.join();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
DAF::LightsOutSolutionAnimator::StartAnimation()
{
    if ( _threadAnimator.joinable() )
        return;
    
    _bStopAnimationRequest = false;
    _bAnimationHasEnded = false;
    _bConsumerNeedsNextFrame = true;
    
    _kpLightsOutSolutionAnimatorSink->AnimationHasStarted();

    _threadAnimator = std::thread(&DAF::LightsOutSolutionAnimator::AnimationThread, this);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
DAF::LightsOutSolutionAnimator::StopAnimation()
{
    if ( ! _threadAnimator.joinable() )
        return;
    
    _mutexAnimator.lock();
    _bStopAnimationRequest = true;
    _mutexAnimator.unlock();
    _conditionVariableAnimator.notify_all();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool
DAF::LightsOutSolutionAnimator::UpdateFrameDelta(double rFrameDeltaSeconds)
{
    _mutexAnimator.lock();
    
    if ( _bAnimationHasEnded )
    {
        _mutexAnimator.unlock();
        _kpLightsOutSolutionAnimatorSink->AnimationHasEnded();
        return false;
    }
    
    _rNextToggleFrameDeltaSeconds -= rFrameDeltaSeconds;
    if ( _bConsumerNeedsNextFrame || _rNextToggleFrameDeltaSeconds > 0.0 )
    {
        _mutexAnimator.unlock();
        return false;
    }

    _mutexAnimator.unlock();
    _kpLightsOutSolutionAnimatorSink->ToggleStateOfElements(_kvuToggleElementIndices);
    _mutexAnimator.lock();
    
    _kvuToggleElementIndices.clear();
    
    _bConsumerNeedsNextFrame = true;
    _mutexAnimator.unlock();
    _conditionVariableAnimator.notify_all();
    
    return true;
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
        _conditionVariableAnimator.wait(lock,
                                        [this]() -> bool
                                        {
                                            return _bConsumerNeedsNextFrame || _bStopAnimationRequest;
                                        });
        
        if ( _bStopAnimationRequest )
            break;
        
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
            _conditionVariableAnimator.wait(lock,
                                            [this]() -> bool
                                            {
                                                return _bConsumerNeedsNextFrame || _bStopAnimationRequest;
                                            });
            
            if ( _bStopAnimationRequest )
                break;
            
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

    const long long kllSleepMilliseconds = _krAnimationDoneSleepSeconds * 1000;
    _conditionVariableAnimator.wait_until(lock,
                                          std::chrono::high_resolution_clock::now() +
                                              std::chrono::milliseconds(kllSleepMilliseconds),
                                          [this]() -> bool
                                          {
                                              return _bStopAnimationRequest;
                                          });

    _bAnimationHasEnded = true;
}
