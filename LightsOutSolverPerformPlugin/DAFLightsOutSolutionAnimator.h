//
//  DAFLightsOutSolutionAnimator.h
//  LightsOut
//
//  Created by David Flores on 5/11/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef DAFLightsOutSolutionAnimator_h
#define DAFLightsOutSolutionAnimator_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace DAF
{
    class ILightsOutSolutionAnimatorSink
    {
    public:
        // ILightsOutSolutionAnimatorSink
        virtual void AnimationHasStarted() = 0;
        virtual void AnimationHasEnded() = 0;
        virtual void ToggleStateOfElements(const std::vector<std::size_t>& kvuToggleElementIndices) = 0;
    };
    
    class LightsOutSolutionAnimator
    {
    public:
        // LightsOutSolutionAnimator
        LightsOutSolutionAnimator(ILightsOutSolutionAnimatorSink* pLightsOutSolutionAnimatorSink,
                                  double rToggleSelectionFrameDeltaSeconds,
                                  double rToggleDominoFrameDeltaSeconds,
                                  double rAnimationDoneSleepSeconds,
                                  const std::vector<bool>& kvbOptimalSolutionMatrix);
        ~LightsOutSolutionAnimator();
        
        void StartAnimation();
        void UpdateFrameDelta(double rFrameDeltaSeconds);
        
    private:
        // LightsOutSolutionAnimator
        void AnimationThread();
        
        ILightsOutSolutionAnimatorSink* const _kpLightsOutSolutionAnimatorSink;
        const double _krToggleSelectionFrameDeltaSeconds;
        const double _krToggleDominoFrameDeltaSeconds;
        const double _krAnimationDoneSleepSeconds;
        const std::vector<bool> _kvbOptimalSolutionMatrix;
        
        std::thread _threadAnimator;
        std::mutex _mutexAnimator;

        double _rNextToggleFrameDeltaSeconds;
        std::vector<std::size_t> _kvuToggleElementIndices;

        bool _bAnimationHasEnded;
        bool _bConsumerNeedsNextFrame;
        std::condition_variable _conditionVariableAnimator;
    };
};

#endif
