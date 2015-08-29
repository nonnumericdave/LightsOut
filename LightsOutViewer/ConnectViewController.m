//
//  ConnectViewController.m
//  LightsOut
//
//  Created by David Flores on 8/28/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#import "ConnectViewController.h"

@implementation ConnectViewController {
    UIImageView *_connectImageView;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _connectImageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Connect.png"]];
    [self.view addSubview:_connectImageView];
}

- (void)viewWillLayoutSubviews {
    [super viewWillLayoutSubviews];
    
    CGSize viewSize = self.view.bounds.size;
    CGRect imageFrame = _connectImageView.frame;
    imageFrame.origin.x = roundf((viewSize.width - imageFrame.size.width)/2.0f);
    imageFrame.origin.y = roundf((viewSize.height - imageFrame.size.height)/2.0f);
    _connectImageView.frame = imageFrame;
}

@end
