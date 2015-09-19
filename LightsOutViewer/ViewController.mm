//
//  ViewController.mm
//  LightsOut
//
//  Created by David Flores on 9/19/15.
//  Copyright © 2015 David Flores. All rights reserved.
//

#include "ViewController.h"

#include "DAFLightsOutBoardView.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation ViewController
{
    CvVideoCamera* _pVideoCamera;
    DAFLightsOutBoardView* _pLightsOutBoardView;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processImage:(cv::Mat&)matImage
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _pVideoCamera = [[CvVideoCamera alloc] initWithParentView:self.videoCameraParentView];
    _pVideoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionBack;
    _pVideoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset1280x720;
    _pVideoCamera.defaultAVCaptureVideoOrientation = AVCaptureVideoOrientationPortrait;
    _pVideoCamera.defaultFPS = 30;
    _pVideoCamera.grayscaleMode = NO;
    _pVideoCamera.delegate = self;
    
    _pLightsOutBoardView = [[DAFLightsOutBoardView alloc] init];
    
    [self.view addSubview:_pLightsOutBoardView];
    [self.view bringSubviewToFront:_pLightsOutBoardView];
    
    _pLightsOutBoardView.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSArray* pHorizontalLayoutConstraintArray =
        [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-0-[_pLightsOutBoardView]-0-|"
                                                options:0
                                                metrics:nil
                                                  views:NSDictionaryOfVariableBindings(_pLightsOutBoardView)];
    
    NSLayoutConstraint* pHeightLayoutConstraint =
        [NSLayoutConstraint constraintWithItem:_pLightsOutBoardView
                                     attribute:NSLayoutAttributeHeight
                                     relatedBy:NSLayoutRelationEqual
                                        toItem:_pLightsOutBoardView
                                     attribute:NSLayoutAttributeWidth
                                    multiplier:1.0
                                      constant:0];
    
    NSLayoutConstraint* pCenterYLayoutConstraint =
        [NSLayoutConstraint constraintWithItem:_pLightsOutBoardView
                                     attribute:NSLayoutAttributeCenterY
                                     relatedBy:NSLayoutRelationEqual
                                        toItem:self.view
                                     attribute:NSLayoutAttributeCenterY
                                    multiplier:1.0
                                      constant:0];
    
    NSArray* pLayoutConstraintArray =
        [pHorizontalLayoutConstraintArray arrayByAddingObjectsFromArray:@[pHeightLayoutConstraint, pCenterYLayoutConstraint]];
    
    [self.view addConstraints:pLayoutConstraintArray];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)viewDidAppear:(BOOL)boolAnimated
{
    [_pVideoCamera start];
}

@end
