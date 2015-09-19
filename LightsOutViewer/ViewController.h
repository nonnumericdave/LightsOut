//
//  ViewController.h
//  LightsOut
//
//  Created by David Flores on 9/19/15.
//  Copyright © 2015 David Flores. All rights reserved.
//

#ifndef ViewController_h
#define ViewController_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface ViewController : UIViewController <CvVideoCameraDelegate>

// CvVideoCameraDelegate
- (void)processImage:(cv::Mat&)matImage;

// UIViewController
@property (strong, nonatomic) IBOutlet UIView *videoCameraParentView;

- (void)viewDidLoad;
- (void)viewDidAppear:(BOOL)boolAnimated;

@end

#endif
