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

// DAFLightsOutBoardView

/*!
 @property initialBoardState
 @brief An NSArray containing N * N NSNumber instances of BOOL, representing the initial state of a game board of dimension N.
 @details When solving the game board, settings this property will cause the in-progress solution to be canceled.
 */
@property (atomic, readwrite, copy) NSArray* initialBoardState;

/*!
 @property isSolving
 @brief If a solution is in progress, including animation, set to YES, otherwise NO.
 */
@property (atomic, readonly) BOOL isSolving;

@end

#endif
