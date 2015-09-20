//
//  DAFLightsOutBoardView.h
//  LightsOut
//
//  Created by David Flores on 9/19/15.
//  Copyright © 2015 David Flores. All rights reserved.
//

#ifndef DAFLightsOutBoardView_h
#define DAFLightsOutBoardView_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFLightsOutBoardView : UIView

// NSObject
- (void)dealloc;

// UIView
- (nonnull instancetype)initWithFrame:(CGRect)rectFrame;
- (void)layoutSubviews;

// DAFLightsOutBoardView

/*!
 @property initialBoardState
 @brief An NSArray containing N * N NSNumber instances of BOOL, representing the initial state of a game board of dimension N.
 @details When solving the game board, settings this property will cause the in-progress solution to be canceled.
 */
@property (atomic, readwrite, copy, nonnull) NSArray<NSNumber*>* initialBoardState;

/*!
 @property isSolving
 @brief If a solution is in progress, including animation, set to YES, otherwise NO.
 */
@property (atomic, readonly) BOOL isSolving;

/*!
 @method solveFromInitialBoardState
 @brief Attempts to solve the game from the initial board state.
 @details Attempts to solve the game from the initial board state, animating the steps of the solution.  
 If a solution is already in progress, it will be cancled and restarted.
 If a solution does not exist, the view will reflect this result.  
 Thread safe.
 */
- (void)solveFromInitialBoardState;

@end

#endif
