//
//  DAFLightsOutRecognizer.cpp
//  LightsOut
//
//  Created by David Flores on 8/29/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#include "DAFLightsOutRecognizer.h"
#include "DAFImplicitHeap.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class AutoLogger
{
public:
    AutoLogger(const std::string kszFunctionName,
               const DAF::LoggingFunction& kloggingFunction);
    ~AutoLogger();
    
private:
    const std::string _kszFunctionName;
    
    // Incredibly dangerous, but it avoids a copy and this class is private.
    const DAF::LoggingFunction& _kloggingFunction;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AutoLogger::AutoLogger(const std::string kszFunctionName,
                       const DAF::LoggingFunction& kloggingFunction) :
    _kszFunctionName(kszFunctionName),
    _kloggingFunction(kloggingFunction)
{
    if ( _kloggingFunction == nullptr )
        return;
    
    _kloggingFunction(_kszFunctionName,
                      "Entering Function",
                      nullptr,
                      4);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AutoLogger::~AutoLogger()
{
    if ( _kloggingFunction == nullptr )
        return;
    
    _kloggingFunction(_kszFunctionName,
                      "Exiting Function",
                      nullptr,
                      4);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
cv::Mat
PreprocessImageForConvexQuadrilaterals(const cv::Mat& kmatImage,
                                       const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    cv::Mat matPreprocessedImage;
    
    cv::cvtColor(kmatImage, matPreprocessedImage, CV_BGR2GRAY);
    
    cv::GaussianBlur(matPreprocessedImage, matPreprocessedImage, cv::Size(3, 3), 0, 0);
    
    cv::Canny(matPreprocessedImage, matPreprocessedImage, 50, 255);
    
    cv::dilate(matPreprocessedImage, matPreprocessedImage, cv::Mat(), cv::Point(-1, -1));
    
    if ( kloggingFunction != nullptr )
        kloggingFunction(__PRETTY_FUNCTION__,
                         "Preprocessed Image",
                         &matPreprocessedImage,
                         3);
    
    return matPreprocessedImage;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
cv::Mat
PreprocessImageForGrid(const cv::Mat& kmatImage,
                       const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    cv::Mat matPreprocessedImage;
    
    cv::cvtColor(kmatImage, matPreprocessedImage, CV_BGR2GRAY);
    
    cv::equalizeHist(matPreprocessedImage, matPreprocessedImage);
    
    cv::GaussianBlur(matPreprocessedImage, matPreprocessedImage, cv::Size(7, 7), 0, 0);
    
    cv::adaptiveThreshold(matPreprocessedImage,
                          matPreprocessedImage,
                          255,
                          cv::ADAPTIVE_THRESH_MEAN_C,
                          cv::THRESH_BINARY,
                          5,
                          2);
    
    cv::bitwise_not(matPreprocessedImage, matPreprocessedImage);
    
    cv::Mat matKernel = (cv::Mat_<uchar>(3,3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
    
    cv::dilate(matPreprocessedImage, matPreprocessedImage, matKernel);
    
    if ( kloggingFunction != nullptr )
        kloggingFunction(__PRETTY_FUNCTION__,
                         "Preprocessed Grid Image",
                         &matPreprocessedImage,
                         3);
    
    cv::Size_<std::size_t> sizePreprocessed = matPreprocessedImage.size();
    
    int iMaxArea = 0;
    cv::Point pointMax;
    for (std::size_t uY = 0; uY < sizePreprocessed.height; ++uY)
    {
        uchar* pucPreprocessedImageRow = matPreprocessedImage.ptr(static_cast<int>(uY));
        
        for (std::size_t uX = 0; uX < sizePreprocessed.width; ++uX)
        {
            if ( pucPreprocessedImageRow[uX] > 127 )
            {
                cv::Point point(static_cast<int>(uX), static_cast<int>(uY));
                
                int iArea =
                    cv::floodFill(matPreprocessedImage,
                                  point,
                                  CV_RGB(64, 64, 64));
                
                if ( iArea > iMaxArea )
                {
                    iMaxArea = iArea;
                    pointMax = point;
                }
            }
        }
    }
    
    cv::floodFill(matPreprocessedImage, pointMax, CV_RGB(255, 255, 255));
    
    for (std::size_t uY = 0; uY < sizePreprocessed.height; ++uY)
    {
        uchar* pucPreprocessedImageRow = matPreprocessedImage.ptr(static_cast<int>(uY));
        
        for (std::size_t uX = 0; uX < sizePreprocessed.width; ++uX)
        {
            if ( pucPreprocessedImageRow[uX] == 64 )
            {
                cv::Point point(static_cast<int>(uX), static_cast<int>(uY));
                cv::floodFill(matPreprocessedImage, point, CV_RGB(0, 0, 0));
            }
        }
    }
    
    cv::erode(matPreprocessedImage, matPreprocessedImage, matKernel);
    
    if ( kloggingFunction != nullptr )
        kloggingFunction(__PRETTY_FUNCTION__,
                         "Flooded Preprocessed Grid Image",
                         &matPreprocessedImage,
                         1);
    
    return matPreprocessedImage;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::vector<std::vector<cv::Point> >
IdentifyConvexQuadrilateralsInImage(const cv::Mat& kmatImage,
                                    const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    cv::Mat matContoursImage =
        PreprocessImageForConvexQuadrilaterals(kmatImage, kloggingFunction);
    
    std::vector<std::vector<cv::Point> > vvpointContours;
    std::vector<cv::Vec4i> vv4Hierarchy;
    cv::findContours(matContoursImage, vvpointContours, vv4Hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    
    std::vector<std::vector<cv::Point> > vvpointQuadrilaterals;
    for (size_t uIndex = 0; uIndex < vv4Hierarchy.size(); ++uIndex)
    {
        if ( vv4Hierarchy[uIndex][3] < 0 )
        {
            std::vector<cv::Point> vpointPolyContour;
            cv::approxPolyDP(vvpointContours[uIndex],
                             vpointPolyContour,
                             cv::arcLength(vvpointContours[uIndex], true) * 0.02,
                             true);
            
            if ( vpointPolyContour.size() == 4 &&
                 ::fabs(cv::contourArea(vpointPolyContour)) > 1000 &&
                 cv::isContourConvex(vpointPolyContour) )
                 vvpointQuadrilaterals.push_back(vpointPolyContour);
        }
    }
    
    if ( kloggingFunction != nullptr )
    {
        std::ostringstream szsMessage("Quadrilateral Count ", std::ios_base::ate);
        szsMessage << vvpointQuadrilaterals.size();
        kloggingFunction(__PRETTY_FUNCTION__,
                         szsMessage.str(),
                         &matContoursImage,
                         2);
    }
    
    return vvpointQuadrilaterals;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::vector<cv::Mat>
WarpConvexQuadrilateralsInImageToSquareImages(const cv::Mat& kmatImage,
                                              const std::vector<std::vector<cv::Point> >& kvvpointConvexQuadrilaterals,
                                              const float krExpansionPercentage,
                                              const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    std::vector<cv::Mat> vmatSquareImages;
    
    for (const std::vector<cv::Point>& kvpointConvexQuadrilateral : kvvpointConvexQuadrilaterals)
    {
        std::vector<cv::Point2f> vpointOrdered(4);
        std::vector<int> viPointSums;
        std::vector<int> viPointDifferences;
        for (const cv::Point& kpoint : kvpointConvexQuadrilateral)
        {
            viPointSums.push_back(kpoint.x + kpoint.y);
            viPointDifferences.push_back(kpoint.y - kpoint.x);
        }
        
        auto pairMinMaxSums = std::minmax_element(viPointSums.begin(), viPointSums.end());
        std::size_t uMinSumIndex = std::distance(viPointSums.begin(), pairMinMaxSums.first);
        std::size_t uMaxSumIndex = std::distance(viPointSums.begin(), pairMinMaxSums.second);
        
        auto pairMinMaxDifferences = std::minmax_element(viPointDifferences.begin(), viPointDifferences.end());
        std::size_t uMinDifferenceIndex = std::distance(viPointDifferences.begin(), pairMinMaxDifferences.first);
        std::size_t uMaxDifferenceIndex = std::distance(viPointDifferences.begin(), pairMinMaxDifferences.second);
        
        vpointOrdered[0] = kvpointConvexQuadrilateral[uMinSumIndex];
        vpointOrdered[1] = kvpointConvexQuadrilateral[uMinDifferenceIndex];
        vpointOrdered[2] = kvpointConvexQuadrilateral[uMaxSumIndex];
        vpointOrdered[3] = kvpointConvexQuadrilateral[uMaxDifferenceIndex];
        
        const double krTopLength = cv::norm(vpointOrdered[1] - vpointOrdered[0]);
        const double krBottomLength = cv::norm(vpointOrdered[2] - vpointOrdered[3]);
        const double krLeftLength = cv::norm(vpointOrdered[3] - vpointOrdered[0]);
        const double krRightLength = cv::norm(vpointOrdered[2] - vpointOrdered[1]);
        
        const double krNormalizedLength =
            std::max(std::max(std::max(krTopLength, krBottomLength), krLeftLength), krRightLength);
        
        std::vector<cv::Point2f> vpointNormalized({
            cv::Point2f(0, 0),
            cv::Point2f(krNormalizedLength, 0),
            cv::Point2f(krNormalizedLength, krNormalizedLength),
            cv::Point2f(0, krNormalizedLength)
        });
        
        if ( krExpansionPercentage != 0 )
        {
            cv::Mat matReversePerspectiveTransform = cv::getPerspectiveTransform(vpointNormalized, vpointOrdered);
            
            const double krExpansionLength = krExpansionPercentage * krNormalizedLength;
            const double krNormalizedExpandedLength = krNormalizedLength + krExpansionLength;
            
            std::vector<cv::Point2f> vpointNormalizedExpanded({
                cv::Point2f(-krExpansionLength, -krExpansionLength),
                cv::Point2f(krNormalizedExpandedLength, -krExpansionLength),
                cv::Point2f(krNormalizedExpandedLength, krNormalizedExpandedLength),
                cv::Point2f(-krExpansionLength, krNormalizedExpandedLength)
            });
            
            cv::perspectiveTransform(vpointNormalizedExpanded, vpointOrdered, matReversePerspectiveTransform);
        }
        
        cv::Mat matWarpedImage;
        cv::Mat matPerspectiveTransform = cv::getPerspectiveTransform(vpointOrdered, vpointNormalized);
        cv::warpPerspective(kmatImage,
                            matWarpedImage,
                            matPerspectiveTransform,
                            cv::Size(krNormalizedLength,
                                     krNormalizedLength));
        
        vmatSquareImages.push_back(matWarpedImage);
        
        if ( kloggingFunction != nullptr )
            kloggingFunction(__PRETTY_FUNCTION__,
                             "Warped Image",
                             &matWarpedImage,
                             1);
    }
    
    return vmatSquareImages;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
void
IdentifyClusterForLine(const cv::Vec2f& vrRhoThetaLine,
                       const float krRhoDeltaThreshold,
                       std::vector<std::vector<cv::Vec2f> >& vvvrRhoThetaClusters,
                       const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    for (std::vector<cv::Vec2f>& vvrRhoThetaCluster : vvvrRhoThetaClusters)
    {
        for (const cv::Vec2f& kvrRhoThetaLine : vvrRhoThetaCluster)
        {
            const float krLineDistance = kvrRhoThetaLine[0];
            
            if ( std::fabs(vrRhoThetaLine[0] - krLineDistance) < krRhoDeltaThreshold )
            {
                vvrRhoThetaCluster.push_back(vrRhoThetaLine);
                
                return;
            }
        }
    }
    
    vvvrRhoThetaClusters.push_back(std::vector<cv::Vec2f>({vrRhoThetaLine}));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::vector<cv::Vec2f>
AverageClustersToLines(std::vector<std::vector<cv::Vec2f> >& vvvrRhoThetaClusters,
                       const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    std::vector<cv::Vec2f> vvrRhoThetaClusterAverages;
    
    for (const std::vector<cv::Vec2f>& kvvrRhoThetaCluster : vvvrRhoThetaClusters)
    {
        cv::Vec2f vrRhoThetaClusterAverage(0.0, 0.0);
        
        for (const cv::Vec2f& kvrVerticalRhoThetaLine : kvvrRhoThetaCluster)
        {
            vrRhoThetaClusterAverage[0] += kvrVerticalRhoThetaLine[0];
            vrRhoThetaClusterAverage[1] += kvrVerticalRhoThetaLine[1];
        }
        
        std::size_t uClusterElementCount = kvvrRhoThetaCluster.size();
        
        vrRhoThetaClusterAverage[0] /= uClusterElementCount;
        vrRhoThetaClusterAverage[1] /= uClusterElementCount;
        
        vvrRhoThetaClusterAverages.push_back(vrRhoThetaClusterAverage);
    }
    
    std::sort(vvrRhoThetaClusterAverages.begin(),
              vvrRhoThetaClusterAverages.end(),
              [](cv::Vec2f vrLHS, cv::Vec2f vrRHS) -> bool
              {
                  return vrLHS[0] < vrRHS[0];
              });
    
    return vvrRhoThetaClusterAverages;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::vector<std::vector<cv::Point> >
IdentifyIntersectionsInGrid(const std::vector<cv::Vec2f>& kvvrHorizontalRhoThetaLines,
                            const std::vector<cv::Vec2f>& kvvrVerticalRhoThetaLines,
                            const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    const std::size_t kuHorizontalLineCount = kvvrHorizontalRhoThetaLines.size();
    const std::size_t kuVerticalLineCount = kvvrVerticalRhoThetaLines.size();
    
    std::vector<std::vector<cv::Point> > vvpointGridIntersections(kuHorizontalLineCount);
    for (std::vector<cv::Point>& vpointVerticalIntersections : vvpointGridIntersections)
        vpointVerticalIntersections.resize(kuVerticalLineCount);
    
    for (std::size_t uVerticalLineIndex = 0;
         uVerticalLineIndex < kuVerticalLineCount;
         ++uVerticalLineIndex)
    {
        const float krVerticalRho = kvvrVerticalRhoThetaLines[uVerticalLineIndex][0];
        const float krVerticalTheta = kvvrVerticalRhoThetaLines[uVerticalLineIndex][1];
        
        const double krCosVerticalTheta = std::cos(krVerticalTheta);
        const double krSinVerticalTheta = std::sin(krVerticalTheta);

        const double krInitialVerticalX = krCosVerticalTheta * krVerticalRho;
        const double krInitialVerticalY = krSinVerticalTheta * krVerticalRho;
        
        const cv::Point kpointVerticalA(krInitialVerticalX - 1000 * krSinVerticalTheta,
                                        krInitialVerticalY + 1000 * krCosVerticalTheta);
        const cv::Point kpointVerticalB(krInitialVerticalX + 1000 * krSinVerticalTheta,
                                        krInitialVerticalY - 1000 * krCosVerticalTheta);
        
        for (std::size_t uHorizontalLineIndex = 0;
             uHorizontalLineIndex < kuHorizontalLineCount;
             ++uHorizontalLineIndex)
        {
            const float krHorizontalRho = kvvrHorizontalRhoThetaLines[uHorizontalLineIndex][0];
            const float krHorizontalTheta = kvvrHorizontalRhoThetaLines[uHorizontalLineIndex][1];

            const double krCosHorizontalTheta = std::cos(krHorizontalTheta);
            const double krSinHorizontalTheta = std::sin(krHorizontalTheta);

            const double krInitialHorizontalX = krCosHorizontalTheta * krHorizontalRho;
            const double krInitialHorizontalY = krSinHorizontalTheta * krHorizontalRho;

            const cv::Point kpointHorizontalA(krInitialHorizontalX - 1000 * krSinHorizontalTheta,
                                              krInitialHorizontalY + 1000 * krCosHorizontalTheta);
            const cv::Point kpointHorizontalB(krInitialHorizontalX + 1000 * krSinHorizontalTheta,
                                              krInitialHorizontalY - 1000 * krCosHorizontalTheta);

            const cv::Point kpointX = kpointHorizontalA - kpointVerticalA;
            const cv::Point kpointA = kpointVerticalB - kpointVerticalA;
            const cv::Point kpointB = kpointHorizontalB - kpointHorizontalA;
            
            const float krCross = kpointA.x * kpointB.y - kpointA.y * kpointB.x;
            
            const cv::Point kpointIntersection =
                kpointVerticalA +
                kpointA *
                ((kpointX.x * kpointB.y - kpointX.y * kpointB.x) / krCross);

            vvpointGridIntersections[uHorizontalLineIndex][uVerticalLineIndex] =
                kpointIntersection;
        }
    }
    
    return vvpointGridIntersections;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::vector<std::vector<cv::Point> >
IdentifyGridInImage(const cv::Mat& kmatImage,
                    const float krExpansionPercentage,
                    const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);

    std::vector<cv::Vec2f> vvrRhoThetaLines;
    cv::HoughLines(kmatImage, vvrRhoThetaLines, 1, CV_PI / 180.0, 150);
    
    const float krThetaDeltaThreshold = CV_PI * 15 / 180.0;
    
    const cv::Size_<float> ksizeImage = kmatImage.size();
    const float krRhoDeltaThreshold = (ksizeImage.width + ksizeImage.height) * krExpansionPercentage / 2.0;
    
    std::vector<std::vector<cv::Vec2f> > vvvrHorizontalRhoThetaClusters;
    std::vector<std::vector<cv::Vec2f> > vvvrVerticalRhoThetaClusters;
    
    for (cv::Vec2f vrRhoThetaLine : vvrRhoThetaLines)
    {
        if ( std::fabs(vrRhoThetaLine[1] - CV_PI / 2.0) < krThetaDeltaThreshold )
        {
            IdentifyClusterForLine(vrRhoThetaLine,
                                   krRhoDeltaThreshold,
                                   vvvrHorizontalRhoThetaClusters,
                                   kloggingFunction);
        }
        else if ( std::fabs(vrRhoThetaLine[1]) < krThetaDeltaThreshold )
        {
            IdentifyClusterForLine(vrRhoThetaLine,
                                   krRhoDeltaThreshold,
                                   vvvrVerticalRhoThetaClusters,
                                   kloggingFunction);
        }
    }
    
    std::vector<cv::Vec2f> vvrHorizontalRhoThetaClusterAverage =
        AverageClustersToLines(vvvrHorizontalRhoThetaClusters,
                               kloggingFunction);
    
    std::vector<cv::Vec2f> vvrVerticalRhoThetaClusterAverage =
        AverageClustersToLines(vvvrVerticalRhoThetaClusters,
                               kloggingFunction);
    
    std::size_t uHorizontalLineClusterCount = vvrHorizontalRhoThetaClusterAverage.size();
    std::size_t uVerticalLineClusterCount = vvrVerticalRhoThetaClusterAverage.size();
    
    if ( kloggingFunction != nullptr )
    {
        std::ostringstream szsMessage("Horizontal Line Cluster Count = ",
                                      std::ios_base::ate);
        szsMessage << uHorizontalLineClusterCount
                   << ", Vertical Line Cluster Count = "
                   << uVerticalLineClusterCount;
        kloggingFunction(__PRETTY_FUNCTION__,
                         szsMessage.str(),
                         &kmatImage,
                         1);
    }
    
    if ( uHorizontalLineClusterCount != uVerticalLineClusterCount ||
         uHorizontalLineClusterCount == 0 )
        return {};
    
    if ( (vvrHorizontalRhoThetaClusterAverage.front()[0] > (krExpansionPercentage * ksizeImage.height + krRhoDeltaThreshold)) ||
         (vvrHorizontalRhoThetaClusterAverage.back()[0] < ((1 - krExpansionPercentage) * ksizeImage.height - krRhoDeltaThreshold)) ||
         (vvrVerticalRhoThetaClusterAverage.front()[0] > (krExpansionPercentage * ksizeImage.width + krRhoDeltaThreshold)) ||
         (vvrVerticalRhoThetaClusterAverage.back()[0] < ((1 - krExpansionPercentage) * ksizeImage.width - krRhoDeltaThreshold)) )
        return {};
    
    std::vector<std::vector<cv::Point> > vvpointGridIntersections =
        IdentifyIntersectionsInGrid(vvrHorizontalRhoThetaClusterAverage,
                                       vvrVerticalRhoThetaClusterAverage,
                                       kloggingFunction);
    
    if ( kloggingFunction != nullptr )
    {
        cv::Mat matGridIntersectionsImage = kmatImage.clone();
        for (std::size_t uRow = 0; uRow < vvpointGridIntersections.size(); ++uRow)
            for (std::size_t uColumn = 0; uColumn < vvpointGridIntersections.size(); ++uColumn)
                cv::circle(matGridIntersectionsImage,
                           vvpointGridIntersections[uRow][uColumn],
                           5,
                           cv::Scalar(128, 128, 128),
                           2);
        
        kloggingFunction(__PRETTY_FUNCTION__,
                         "Grid Intersections",
                         &matGridIntersectionsImage,
                         0);
    }
    
    return vvpointGridIntersections;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
cv::Mat
PreprocessImageForElementState(const cv::Mat& kmatImage,
                               const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    cv::Mat matPreprocessedImage;
    
    cv::cvtColor(kmatImage, matPreprocessedImage, CV_BGR2GRAY);
    
    cv::GaussianBlur(matPreprocessedImage, matPreprocessedImage, cv::Size(7, 7), 0, 0);
    
    cv::adaptiveThreshold(matPreprocessedImage,
                          matPreprocessedImage,
                          255,
                          cv::ADAPTIVE_THRESH_MEAN_C,
                          cv::THRESH_BINARY,
                          5,
                          2);
    
    DAF::ImplicitHeap<int, cv::Point> implicitHeap(3);
    cv::Size_<std::size_t> sizePreprocessed = matPreprocessedImage.size();
    
    for (std::size_t uY = 0; uY < sizePreprocessed.height; ++uY)
    {
        uchar* pucPreprocessedImageRow = matPreprocessedImage.ptr(static_cast<int>(uY));
        
        for (std::size_t uX = 0; uX < sizePreprocessed.width; ++uX)
        {
            if ( pucPreprocessedImageRow[uX] < 128 )
            {
                cv::Point point(static_cast<int>(uX), static_cast<int>(uY));
                
                int iArea =
                    cv::floodFill(matPreprocessedImage,
                                  point,
                                  CV_RGB(64, 64, 64));

                implicitHeap.Insert(iArea, point);
            }
        }
    }
    
    std::vector<cv::Point> vpoint(implicitHeap.Values());
    for (cv::Point& point : vpoint)
        cv::floodFill(matPreprocessedImage, point, CV_RGB(0, 0, 0));
    
    for (std::size_t uY = 0; uY < sizePreprocessed.height; ++uY)
    {
        uchar* pucPreprocessedImageRow = matPreprocessedImage.ptr(static_cast<int>(uY));
        
        for (std::size_t uX = 0; uX < sizePreprocessed.width; ++uX)
        {
            if ( pucPreprocessedImageRow[uX] == 64 )
            {
                cv::Point point(static_cast<int>(uX), static_cast<int>(uY));
                cv::floodFill(matPreprocessedImage, point, CV_RGB(255, 255, 255));
            }
        }
    }
    
    if ( kloggingFunction != nullptr )
        kloggingFunction(__PRETTY_FUNCTION__,
                         "Preprocessed Image",
                         &matPreprocessedImage,
                         3);
    
    return matPreprocessedImage;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
bool
IdentifyElementStateFromSquareImage(const cv::Mat& kmatSquareImage,
                                    const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    cv::Mat matPreprocessedImage =
        PreprocessImageForElementState(kmatSquareImage,
                                       kloggingFunction);
    
    cv::Size_<std::size_t> sizePreprocessed = matPreprocessedImage.size();
    const std::size_t kuCenterXY = sizePreprocessed.height / 2;
    
    std::size_t uStateSum = 0;
    for (std::size_t uY = 0; uY < sizePreprocessed.height; ++uY)
    {
        uchar* pucPreprocessedImageRow = matPreprocessedImage.ptr(static_cast<int>(uY));
        
        for (std::size_t uX = 0; uX < sizePreprocessed.width; ++uX)
        {
            if ( pucPreprocessedImageRow[uX] < 128 )
            {
                const std::size_t kuXDistanceFromCenterX =
                    (uX > kuCenterXY) ?
                        uX - kuCenterXY :
                        kuCenterXY - uX;
                
                const std::size_t kuYDistanceFromCenterY =
                    (uY > kuCenterXY) ?
                        uY - kuCenterXY :
                        kuCenterXY - uY;

                uStateSum +=
                    kuCenterXY -
                    std::max(kuXDistanceFromCenterX, kuYDistanceFromCenterY);
            }
        }
    }
    
    const std::size_t kuThreshold =
        (sizePreprocessed.height * sizePreprocessed.height) * 0.33;

    if ( kloggingFunction != nullptr )
    {
        std::ostringstream szsMessage("Sum = ", std::ios_base::ate);
        szsMessage << uStateSum
                   << ", Minimum Threshold = "
                   << kuThreshold;
        kloggingFunction(__PRETTY_FUNCTION__,
                         szsMessage.str(),
                         &matPreprocessedImage,
                         0);
    }
    
    return uStateSum > kuThreshold;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::pair<std::vector<bool>, std::size_t>
IdentifyStateMatrixFromSquareImage(const cv::Mat& kmatSquareImage,
                                   const float krExpansionPercentage,
                                   const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    cv::Mat matPreprocessedSquareImage =
        PreprocessImageForGrid(kmatSquareImage,
                               kloggingFunction);
    
    std::vector<std::vector<cv::Point> > vvpGridIntersections =
        IdentifyGridInImage(matPreprocessedSquareImage,
                            krExpansionPercentage,
                            kloggingFunction);
    
    const std::size_t kuLineCount = vvpGridIntersections.size();
    if ( kuLineCount < 2 )
        return {{}, 0};
    
    double rMinQuadrilateralArea = std::numeric_limits<double>::max();
    double rMaxQuadrilateralArea = std::numeric_limits<double>::min();
    
    std::vector<std::vector<cv::Point> > vvpointConvexQuadrilaterals;
    for (std::size_t uHorizontalLineIndex = 0;
         uHorizontalLineIndex < (kuLineCount - 1);
         ++uHorizontalLineIndex)
    {
        for (std::size_t uVerticalLineIndex = 0;
             uVerticalLineIndex < (kuLineCount - 1);
             ++uVerticalLineIndex)
        {
            std::vector<cv::Point> vpointConvexQuadrilateral({
                vvpGridIntersections[uHorizontalLineIndex][uVerticalLineIndex],
                vvpGridIntersections[uHorizontalLineIndex][uVerticalLineIndex + 1],
                vvpGridIntersections[uHorizontalLineIndex + 1][uVerticalLineIndex + 1],
                vvpGridIntersections[uHorizontalLineIndex + 1][uVerticalLineIndex]
            });
            
            vvpointConvexQuadrilaterals.push_back(vpointConvexQuadrilateral);
            
            double rQuadrilateralArea = std::fabs(cv::contourArea(vpointConvexQuadrilateral));
            
            if ( rQuadrilateralArea < rMinQuadrilateralArea )
                rMinQuadrilateralArea = rQuadrilateralArea;
            
            if ( rQuadrilateralArea > rMaxQuadrilateralArea )
                rMaxQuadrilateralArea = rQuadrilateralArea;
        }
    }
    
    if ( (rMaxQuadrilateralArea < std::numeric_limits<double>::min()) ||
          ((rMinQuadrilateralArea / rMaxQuadrilateralArea) < 0.5) )
        return {{}, 0};
    
    std::vector<cv::Mat> vmatSquareElementImages =
        WarpConvexQuadrilateralsInImageToSquareImages(kmatSquareImage,
                                                      vvpointConvexQuadrilaterals,
                                                      -krExpansionPercentage,
                                                      kloggingFunction);
    
    std::vector<bool> vbStateMatrix;
    for (const cv::Mat& matSquareElementImage : vmatSquareElementImages)
        vbStateMatrix.push_back(IdentifyElementStateFromSquareImage(matSquareElementImage,
                                                                    kloggingFunction));
    
    const std::size_t kuDimension = kuLineCount - 1;
    return {vbStateMatrix, kuDimension};
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool
DAF::RecognizeLightsOutBoardStateFromImage(const cv::Mat& kmatImage,
                                           std::vector<bool>& vbStateMatrix,
                                           std::size_t& uDimension,
                                           const DAF::LoggingFunction& kloggingFunction)
{
    AutoLogger autoLogger(__PRETTY_FUNCTION__, kloggingFunction);
    
    std::vector<std::vector<cv::Point> > vvpointConvexQuadrilaterals =
        IdentifyConvexQuadrilateralsInImage(kmatImage,
                                            kloggingFunction);
    
    if ( vvpointConvexQuadrilaterals.size() == 0 )
        return false;

    const float krExpansionPercentage = 0.1;
    
    std::vector<cv::Mat> vmatSquareImages =
        WarpConvexQuadrilateralsInImageToSquareImages(kmatImage,
                                                      vvpointConvexQuadrilaterals,
                                                      krExpansionPercentage,
                                                      kloggingFunction);
    
    if ( vmatSquareImages.size() == 0 )
        return false;
    
    for (const cv::Mat& kmatSquareImage : vmatSquareImages)
    {
        std::pair<std::vector<bool>, std::size_t> pvbuStateMatrixAndDimension =
            IdentifyStateMatrixFromSquareImage(kmatSquareImage,
                                               krExpansionPercentage,
                                               kloggingFunction);
        
        const std::size_t kuDimension = pvbuStateMatrixAndDimension.second;
        if ( kuDimension == 0 )
            continue;
        
        vbStateMatrix = pvbuStateMatrixAndDimension.first;
        uDimension = kuDimension;
        return true;
    }

    return false;
}
