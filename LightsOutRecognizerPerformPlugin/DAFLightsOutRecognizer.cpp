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
static
cv::Mat
PreprocessImageForConvexQuadrilaterals(const cv::Mat& kmatImage)
{
	cv::Mat matPreprocessedImage;
	
	cv::cvtColor(kmatImage, matPreprocessedImage, CV_BGR2GRAY);
	
	cv::GaussianBlur(matPreprocessedImage, matPreprocessedImage, cv::Size(3, 3), 0, 0);
	
	cv::Canny(matPreprocessedImage, matPreprocessedImage, 50, 255);
	
	cv::dilate(matPreprocessedImage, matPreprocessedImage, cv::Mat(), cv::Point(-1, -1));
	
	return matPreprocessedImage;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
cv::Mat
PreprocessImageForGrid(const cv::Mat& kmatImage)
{
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
	
	return matPreprocessedImage;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::vector<std::vector<cv::Point> >
IdentifyConvexQuadrilateralsInImage(const cv::Mat& kmatImage)
{
	cv::Mat matContoursImage = PreprocessImageForConvexQuadrilaterals(kmatImage);
	
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
	
	return vvpointQuadrilaterals;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::vector<cv::Mat>
WarpConvexQuadrilateralsInImageToSquareImages(const cv::Mat& kmatImage,
											  const std::vector<std::vector<cv::Point> >& kvvpointConvexQuadrilaterals,
											  const float krExpansionPercentage)
{
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
	}
	
	return vmatSquareImages;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
void
IdentifyClusterForLine(const cv::Vec2f& vrRhoThetaLine,
					   const float krRhoDeltaThreshold,
					   std::vector<std::vector<cv::Vec2f> >& vvvrRhoThetaClusters)
{
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
AverageClustersToLines(std::vector<std::vector<cv::Vec2f> >& vvvrRhoThetaClusters)
{
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
							const std::vector<cv::Vec2f>& kvvrVerticalRhoThetaLines)
{
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
		
		if ( krVerticalTheta == 0 )
		{
			for (std::size_t uHorizontalLineIndex = 0;
				 uHorizontalLineIndex < kuHorizontalLineCount;
				 ++uHorizontalLineIndex)
			{
				const float krHorizontalRho = kvvrHorizontalRhoThetaLines[uHorizontalLineIndex][0];
				const float krHorizontalTheta = kvvrHorizontalRhoThetaLines[uHorizontalLineIndex][1];
				
				const float krX = krVerticalRho;
				
				const float krY =
					std::cos(krHorizontalTheta) / std::sin(krHorizontalTheta) * krX +
					krHorizontalRho *
						(std::sin(krHorizontalTheta) -
						 std::cos(krHorizontalTheta) / std::sin(krHorizontalTheta) * std::cos(krHorizontalTheta));
				
				vvpointGridIntersections[uHorizontalLineIndex][uVerticalLineIndex] =
					cv::Point(krX, krY);
			}
		}
		else
		{
			for (std::size_t uHorizontalLineIndex = 0;
				 uHorizontalLineIndex < kuHorizontalLineCount;
				 ++uHorizontalLineIndex)
			{
				const float krHorizontalRho = kvvrHorizontalRhoThetaLines[uHorizontalLineIndex][0];
				const float krHorizontalTheta = kvvrHorizontalRhoThetaLines[uHorizontalLineIndex][1];
				
				const float krX =
					(krVerticalRho * std::cos(2 * krVerticalTheta) / std::sin(krVerticalTheta) -
					 krHorizontalRho * std::cos(2 * krHorizontalTheta) / std::sin(krHorizontalTheta)) /
					(std::cos(krVerticalTheta) / std::sin(krVerticalTheta) -
					 std::cos(krHorizontalTheta) / std::sin(krHorizontalTheta));
				
				const float krY =
					(krVerticalRho * std::cos(2 * krVerticalTheta) * std::cos(krHorizontalTheta) /
					 (std::sin(krVerticalTheta) * std::sin(krHorizontalTheta)) -
					 krHorizontalRho * std::cos(2 * krHorizontalTheta) * std::cos(krVerticalTheta)/
					 (std::sin(krHorizontalTheta) * std::sin(krVerticalTheta))) /
					(std::cos(krVerticalTheta) / std::sin(krVerticalTheta) -
					 std::cos(krHorizontalTheta) / std::sin(krHorizontalTheta));
				
				vvpointGridIntersections[uHorizontalLineIndex][uVerticalLineIndex] =
					cv::Point(krX, krY);
			}
		}
	}
	
	return vvpointGridIntersections;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::vector<std::vector<cv::Point> >
IdentifyGridInImage(const cv::Mat& kmatImage, const float krExpansionPercentage)
{
	std::vector<cv::Vec2f> vvrRhoThetaLines;
	cv::HoughLines(kmatImage, vvrRhoThetaLines, 1, CV_PI / 180.0, 175);
	
	const float krThetaDeltaThreshold = CV_PI * 5 / 180.0;
	
	const cv::Size_<float> ksizeImage = kmatImage.size();
	const float krRhoDeltaThreshold = (ksizeImage.width + ksizeImage.height) * krExpansionPercentage / 2.0;
	
	std::vector<std::vector<cv::Vec2f> > vvvrHorizontalRhoThetaClusters;
	std::vector<std::vector<cv::Vec2f> > vvvrVerticalRhoThetaClusters;
	
	for (cv::Vec2f vrRhoThetaLine : vvrRhoThetaLines)
	{
		if ( std::fabs(vrRhoThetaLine[1] - CV_PI / 2.0) < krThetaDeltaThreshold )
		{
			IdentifyClusterForLine(vrRhoThetaLine, krRhoDeltaThreshold, vvvrHorizontalRhoThetaClusters);
		}
		else if ( std::fabs(vrRhoThetaLine[1]) < krThetaDeltaThreshold )
		{
			IdentifyClusterForLine(vrRhoThetaLine, krRhoDeltaThreshold, vvvrVerticalRhoThetaClusters);
		}
	}
	
	std::vector<cv::Vec2f> vvrHorizontalRhoThetaClusterAverage =
		AverageClustersToLines(vvvrHorizontalRhoThetaClusters);
	
	std::vector<cv::Vec2f> vvrVerticalRhoThetaClusterAverage =
		AverageClustersToLines(vvvrVerticalRhoThetaClusters);
	
	std::size_t uHorizontalLineClusterCount = vvrHorizontalRhoThetaClusterAverage.size();
	std::size_t uVerticalLineClusterCount = vvrVerticalRhoThetaClusterAverage.size();
	
	if ( uHorizontalLineClusterCount != uVerticalLineClusterCount ||
		uHorizontalLineClusterCount == 0 )
		return {};
	
	if ( (vvrHorizontalRhoThetaClusterAverage.front()[0] > (krExpansionPercentage * ksizeImage.height + krRhoDeltaThreshold)) ||
		 (vvrHorizontalRhoThetaClusterAverage.back()[0] < ((1 - krExpansionPercentage) * ksizeImage.height - krRhoDeltaThreshold)) ||
		 (vvrVerticalRhoThetaClusterAverage.front()[0] > (krExpansionPercentage * ksizeImage.width + krRhoDeltaThreshold)) ||
		 (vvrVerticalRhoThetaClusterAverage.back()[0] < ((1 - krExpansionPercentage) * ksizeImage.width - krRhoDeltaThreshold)) )
		return {};
	
	return IdentifyIntersectionsInGrid(vvrHorizontalRhoThetaClusterAverage, vvrVerticalRhoThetaClusterAverage);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
cv::Mat
PreprocessImageForElementState(const cv::Mat& kmatImage)
{
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
	
	return matPreprocessedImage;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
bool
IdentifyElementStateFromSquareImage(const cv::Mat& kmatSquareImage)
{
	cv::Mat matPreprocessedImage =
		PreprocessImageForElementState(kmatSquareImage);
	
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

	return uStateSum > kuThreshold;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
std::pair<std::vector<bool>, std::size_t>
IdentifyStateMatrixFromSquareImage(const cv::Mat& kmatSquareImage, const float krExpansionPercentage)
{
	cv::Mat matPreprocessedSquareImage = PreprocessImageForGrid(kmatSquareImage);
	
	std::vector<std::vector<cv::Point> > vvpGridIntersections =
		IdentifyGridInImage(matPreprocessedSquareImage, krExpansionPercentage);
	
	const std::size_t kuLineCount = vvpGridIntersections.size();
	if ( kuLineCount < 2 )
		return {{}, 0};
	
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
		}
	}
	
	std::vector<cv::Mat> vmatSquareElementImages =
		WarpConvexQuadrilateralsInImageToSquareImages(kmatSquareImage,
													  vvpointConvexQuadrilaterals,
													  -krExpansionPercentage);
	
	std::vector<bool> vbStateMatrix;
	for (const cv::Mat& matSquareElementImage : vmatSquareElementImages)
		vbStateMatrix.push_back(IdentifyElementStateFromSquareImage(matSquareElementImage));
	
	const std::size_t kuDimension = kuLineCount - 1;
	return {vbStateMatrix, kuDimension};
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool
DAF::RecognizeLightsOutBoardStateFromImage(const cv::Mat& kmatImage,
										   std::vector<bool>& vbStateMatrix,
										   std::size_t& uDimension)
{
	std::vector<std::vector<cv::Point> > vvpointConvexQuadrilaterals =
		IdentifyConvexQuadrilateralsInImage(kmatImage);
	
	if ( vvpointConvexQuadrilaterals.size() == 0 )
		return false;

	const float krExpansionPercentage = 0.1;
	
	std::vector<cv::Mat> vmatSquareImages =
		WarpConvexQuadrilateralsInImageToSquareImages(kmatImage, vvpointConvexQuadrilaterals, krExpansionPercentage);
	
	if ( vmatSquareImages.size() == 0 )
		return false;
	
	for (const cv::Mat& kmatSquareImage : vmatSquareImages)
	{
		std::pair<std::vector<bool>, std::size_t> pvbuStateMatrixAndDimension =
			IdentifyStateMatrixFromSquareImage(kmatSquareImage, krExpansionPercentage);
		
		const std::size_t kuDimension = pvbuStateMatrixAndDimension.second;
		if ( kuDimension == 0 )
			continue;
		
		vbStateMatrix = pvbuStateMatrixAndDimension.first;
		uDimension = kuDimension;
		return true;
	}

	return false;
}
