//
//  DAFOpenCVCamera.m
//  LightsOut
//
//  Created by David Flores on 8/29/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#include "DAFOpenCVCamera.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static DAFOpenCVCamera* __weak g_pSharedOpenCVCamera = nil;
static std::mutex g_mutex;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFOpenCVCamera ()

// NSObject
- (instancetype)init;
- (void)dealloc;

@end

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation DAFOpenCVCamera
{
    CvVideoCamera* _pVideoCamera;
    RIImage* _pImage;
    cv::Mat _matrix;
    std::mutex _mutex;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processImage:(cv::Mat&)matImage
{
    std::unique_lock<std::mutex> lock(::g_mutex);
    if ( ::g_pSharedOpenCVCamera == nil )
        return;
    
    DAFOpenCVCamera* __strong pOpenCVCamera = self;
    
    lock.unlock();
    
    const cv::Size ksize = matImage.size();
    
    cv::Mat matImageProcessed(ksize, CV_8UC4);
    cv::cvtColor(matImage, matImageProcessed, CV_BGR2RGBA);
    
    assert( matImageProcessed.isContinuous() );
    
    RIImage* pImage =
        [[RIImage alloc] initWithData:matImageProcessed.ptr()
                                width:ksize.width
                               height:ksize.height
                                scale:1.0
                     coordinateSystem:RICoordinateSystemTopLeftOrigin
                   premultipliedAlpha:NO
                          pixelFormat:GL_RGBA];

    ::dispatch_async(::dispatch_get_main_queue(),
                    ^void(void)
                    {
                        [pOpenCVCamera willChangeValueForKey:@"image"];
                        [pOpenCVCamera willChangeValueForKey:@"matrix"];

                        std::unique_lock<std::mutex> lock(_mutex);
                        _pImage = pImage;
                        _matrix = matImageProcessed;
                        lock.unlock();
                        
                        [pOpenCVCamera didChangeValueForKey:@"image"];
                        [pOpenCVCamera didChangeValueForKey:@"matrix"];
                    });
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (DAFOpenCVCamera*)sharedOpenCVCamera
{
    std::unique_lock<std::mutex> lock(::g_mutex);
    
    DAFOpenCVCamera* __strong pOpenCVCamera = ::g_pSharedOpenCVCamera;
    if ( pOpenCVCamera == nil )
    {
        pOpenCVCamera = [[DAFOpenCVCamera alloc] init];
        ::g_pSharedOpenCVCamera = pOpenCVCamera;
    }
    
    return pOpenCVCamera;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (RIImage*)image
{
    std::unique_lock<std::mutex> lock(_mutex);
    RIImage* pImage = _pImage;
    lock.unlock();
    
    return pImage;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (cv::Mat)matrix
{
    std::unique_lock<std::mutex> lock(_mutex);
    cv::Mat matrix(_matrix);
    lock.unlock();
    
    return matrix;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (instancetype)init
{
    self = [super init];
    
    if ( self != nil )
    {
        _pVideoCamera = [[CvVideoCamera alloc] init];
        _pVideoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionBack;
        _pVideoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset1280x720;
        _pVideoCamera.defaultAVCaptureVideoOrientation = AVCaptureVideoOrientationPortrait;
        _pVideoCamera.defaultFPS = 10;
        _pVideoCamera.grayscaleMode = NO;
        _pVideoCamera.delegate = self;
        
        [_pVideoCamera start];
    }
    
    return self;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)dealloc
{
    [_pVideoCamera stop];

    _pVideoCamera.delegate = nil;
}

@end
