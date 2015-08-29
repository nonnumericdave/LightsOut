//
//  DAFOpenCVCamera.h
//  LightsOut
//
//  Created by David Flores on 8/29/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef DAFOpenCVCamera_h
#define DAFOpenCVCamera_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface DAFOpenCVCamera : NSObject <CvVideoCameraDelegate>

// CvVideoCameraDelegate
- (void)processImage:(cv::Mat&)matImage;

// DAFOpenCVCamera
+ (DAFOpenCVCamera*)sharedOpenCVCamera;

@property (nonatomic, readonly) RIImage* image;
- (RIImage*)image;

@property (nonatomic, readonly) cv::Mat matrix;
- (cv::Mat)matrix;

@end

#endif
