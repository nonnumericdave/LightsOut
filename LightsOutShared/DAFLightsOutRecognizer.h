//
//  DAFLightsOutRecognizer.h
//  LightsOut
//
//  Created by David Flores on 8/29/15.
//  Copyright (c) 2015 David Flores. All rights reserved.
//

#ifndef DAFLightsOutRecognizer_h
#define DAFLightsOutRecognizer_h

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace DAF
{
    using LoggingFunction = std::function<void(const std::string& kszFunctionName,
                                               const std::string& kszMessage,
                                               const cv::Mat* pkmatImage,
                                               const std::size_t uDebugLevel)>;

    bool RecognizeLightsOutBoardStateFromImage(const cv::Mat& kmatImage,
                                               std::vector<bool>& vbStateMatrix,
                                               std::size_t& uDimension,
                                               const LoggingFunction& kloggingFunction = nullptr);
};

#endif
