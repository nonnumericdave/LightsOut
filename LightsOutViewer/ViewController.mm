//
//  ViewController.mm
//  LightsOut
//
//  Created by David Flores on 9/19/15.
//  Copyright © 2015 David Flores. All rights reserved.
//

#include "ViewController.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation ViewController
{
    CvVideoCamera* _pVideoCamera;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processImage:(cv::Mat&)matImage
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _pVideoCamera = [[CvVideoCamera alloc] initWithParentView:self.view];
    _pVideoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionBack;
    _pVideoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset1280x720;
    _pVideoCamera.defaultAVCaptureVideoOrientation = AVCaptureVideoOrientationPortrait;
    _pVideoCamera.defaultFPS = 30;
    _pVideoCamera.grayscaleMode = NO;
    _pVideoCamera.delegate = self;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)viewDidAppear:(BOOL)boolAnimated
{
    [_pVideoCamera start];
}

@end
