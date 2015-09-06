//
//  DAFLightsOutSolverPatch.h
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef DAFLightsOutSolverPatch_h
#define DAFLightsOutSolverPatch_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutSolverPatch : PMRPatch

// PMRPatch
- (void)processPatchWithContext:(PMRProcessContext*)pProcessContext;

// DAFLightsOutSolverPatch
@property (nonatomic, readonly) PMRArrayInputPort* boardState;
@property (nonatomic, readonly) PMRPrimitiveInputPort* solveBoardPulse;
@property (nonatomic, readonly) PMRPrimitiveOutputPort* solvingBoard;
@property (nonatomic, readonly) PMRArrayOutputPort* boardStateTogglePulse;

@end

#endif
