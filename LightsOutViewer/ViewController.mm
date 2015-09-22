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
static const dispatch_semaphore_t g_dispatchSemaphoreImageProcessing =
    ::dispatch_semaphore_create(3);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@interface ViewController ()

// ViewController
- (void)addConstraintsForSubview:(UIView*)pSubView;
- (void)didTapLightsOutBoardViewWithGestureRecognizer:(UITapGestureRecognizer*)pTapGestureRecgonizer;
- (void)setBoardStateArray:(NSArray*)pBoardStateArray;
- (void)displayLoggingImage:(UIImage*)pLoggingImage;

@end

#if defined(DEBUG)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
LogImageToLayerReleaseData(void* pvInfo, const void* pvData, size_t uSize)
{
    cv::Mat* pmatImage = static_cast<cv::Mat*>(pvInfo);
    delete pmatImage;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void
LogImageToLayer(const cv::Mat& kmatImage,
                ViewController* pViewController)
{
    int iConvertColorCode;
    switch ( kmatImage.type() )
    {
        case CV_8UC1:
            iConvertColorCode = CV_GRAY2RGBA;
            break;
            
        case CV_8UC3:
            iConvertColorCode = CV_BGR2RGBA;
            break;
            
        default:
            return;
    }

    cv::Mat matImageResized;
    int iMaxSizeDimension = std::max(kmatImage.rows, kmatImage.cols);
    if ( iMaxSizeDimension > 768 )
    {
        const double krResizeFactor =
            static_cast<double>(768) / iMaxSizeDimension;
        
        cv::Size size(kmatImage.rows / krResizeFactor,
                      kmatImage.cols / krResizeFactor);
        
        cv::resize(kmatImage, matImageResized, size);
    }
    else
    {
        matImageResized = kmatImage;
    }
    
    cv::Mat* pmatImage = new cv::Mat;
    cv::cvtColor(matImageResized, *pmatImage, iConvertColorCode);
    
    assert( pmatImage->isContinuous() );
    
    CGDataProviderRef dataProviderRef =
        ::CGDataProviderCreateWithData(pmatImage,
                                       pmatImage->ptr(),
                                       pmatImage->total() * pmatImage->elemSize(),
                                       ::LogImageToLayerReleaseData);
    
    CGColorSpaceRef colorSpaceRef =
        ::CGColorSpaceCreateDeviceRGB();
    
    const size_t kuBitsPerComponent = pmatImage->elemSize1() * 8;
    const size_t kuBitsPerPixel = pmatImage->elemSize() * 8;
    const size_t kuBytesPerRow = pmatImage->cols * pmatImage->elemSize();
    
    CGImageRef imageRef =
        ::CGImageCreate(pmatImage->cols,
                        pmatImage->rows,
                        kuBitsPerComponent,
                        kuBitsPerPixel,
                        kuBytesPerRow,
                        colorSpaceRef,
                        kCGBitmapByteOrder32Big |kCGImageAlphaNoneSkipLast,
                        dataProviderRef,
                        NULL,
                        NO,
                        kCGRenderingIntentDefault);

    UIImage* pLoggingImage =
        [[UIImage alloc] initWithCGImage:imageRef];

    ::CGImageRelease(imageRef);
    
    ::CGColorSpaceRelease(colorSpaceRef);
                        
    ::CGDataProviderRelease(dataProviderRef);
    
    [pViewController performSelectorOnMainThread:@selector(displayLoggingImage:)
                                      withObject:pLoggingImage
                                   waitUntilDone:NO];
}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@implementation ViewController
{
    CvVideoCamera* _pVideoCamera;
    DAFLightsOutBoardView* _pLightsOutBoardView;
    UIImageView* _pLoggingImageView;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)processImage:(cv::Mat&)matImage
{
    if ( _pLightsOutBoardView.isSolving )
        return;
    
    bool bCanProcessImage =
        ::dispatch_semaphore_wait(::g_dispatchSemaphoreImageProcessing, 0) == 0;
   
    if ( ! bCanProcessImage )
        return;
    
    const cv::Mat kmatImage = matImage.clone();

    dispatch_block_t dispatchBlock =
        ^void(void)
        {
            std::vector<bool> vbStateMatrix;
            std::size_t uDimension;
            
#if defined(DEBUG)
            const void* pvkKey = kmatImage.ptr();
            
            auto loggingFunction =
                [pvkKey, self]
                (const std::string& kszFunctionName,
                 const std::string& kszMessage,
                 const cv::Mat* pkmatImage,
                 const std::size_t uDebugLevel) -> void
                {
                    if ( uDebugLevel > 3 )
                        return;
                    
                    if ( pkmatImage != nullptr &&
                         kszMessage.find("Flooded Preprocessed Grid Image") != std::string::npos )
                        ::LogImageToLayer(*pkmatImage, self);

                    ::NSLog(@"\n[%p : %@]\n%@",
                            pvkKey,
                            [NSString stringWithUTF8String:kszFunctionName.c_str()],
                            [NSString stringWithUTF8String:kszMessage.c_str()]);
                };
#else
            DAF::LoggingFunction loggingFunction(nullptr);
#endif
            
            bool bDidRecognizeBoardState =
                DAF::RecognizeLightsOutBoardStateFromImage(kmatImage,
                                                           vbStateMatrix,
                                                           uDimension,
                                                           loggingFunction);
            
            if ( bDidRecognizeBoardState )
            {
                const NSUInteger kuBoardElements = vbStateMatrix.size();
                NSMutableArray* pBoardStateArray =
                    [NSMutableArray arrayWithCapacity:kuBoardElements];
                
                for (bool bElementState : vbStateMatrix)
                {
                    NSNumber* pElementStateNumber =
                        [NSNumber numberWithBool:bElementState];
                    
                    [pBoardStateArray addObject:pElementStateNumber];
                }
                
                [self setBoardStateArray:pBoardStateArray];
            }
            
            ::dispatch_semaphore_signal(::g_dispatchSemaphoreImageProcessing);
        };
    
    ::dispatch_async(::dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
                     dispatchBlock);
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

#if defined(DEBUG)
    _pLoggingImageView = [[UIImageView alloc] init];
    _pLoggingImageView.contentMode = UIViewContentModeScaleAspectFit;
    _pLoggingImageView.alpha = 0.5;
    [self.view addSubview:_pLoggingImageView];
    [self.view bringSubviewToFront:_pLoggingImageView];
    [self addConstraintsForSubview:_pLoggingImageView];
#endif
    
    _pLightsOutBoardView = [[DAFLightsOutBoardView alloc] init];
    [self.view addSubview:_pLightsOutBoardView];
    [self.view bringSubviewToFront:_pLightsOutBoardView];
    [self addConstraintsForSubview:_pLightsOutBoardView];
    
    UITapGestureRecognizer* pTapGestureRecognizer =
        [[UITapGestureRecognizer alloc] initWithTarget:self
                                                action:@selector(didTapLightsOutBoardViewWithGestureRecognizer:)];
    
    [_pLightsOutBoardView addGestureRecognizer:pTapGestureRecognizer];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)viewDidAppear:(BOOL)boolAnimated
{
    [_pVideoCamera start];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)addConstraintsForSubview:(UIView*)pSubView
{
    pSubView.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSArray* pHorizontalLayoutConstraintArray =
        [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-0-[pSubView]-0-|"
                                                options:0
                                                metrics:nil
                                                  views:NSDictionaryOfVariableBindings(pSubView)];
    
    NSLayoutConstraint* pHeightLayoutConstraint =
        [NSLayoutConstraint constraintWithItem:pSubView
                                     attribute:NSLayoutAttributeHeight
                                     relatedBy:NSLayoutRelationEqual
                                        toItem:pSubView
                                     attribute:NSLayoutAttributeWidth
                                    multiplier:1.0
                                      constant:0];
    
    NSLayoutConstraint* pCenterYLayoutConstraint =
        [NSLayoutConstraint constraintWithItem:pSubView
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
- (void)didTapLightsOutBoardViewWithGestureRecognizer:(UITapGestureRecognizer*)pTapGestureRecgonizer
{
    assert( pTapGestureRecgonizer.view == _pLightsOutBoardView );
    
    [_pLightsOutBoardView solveFromInitialBoardState];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)setBoardStateArray:(NSArray*)pBoardStateArray
{
    if ( _pLightsOutBoardView.isSolving )
        return;
    
    _pLightsOutBoardView.initialBoardState = pBoardStateArray;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- (void)displayLoggingImage:(UIImage*)pLoggingImage
{
    _pLoggingImageView.image = pLoggingImage;
}

@end
