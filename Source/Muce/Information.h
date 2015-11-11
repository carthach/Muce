/*
  ==============================================================================

    Information.h
    Created: 18 Oct 2015 11:58:19am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#ifndef INFORMATION_H_INCLUDED
#define INFORMATION_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "opencv2/opencv.hpp"

#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>

#include "Extraction.h"

namespace Muce {
    class Information {
    public:
        Information();
        ~Information();
        
        //Conversion to OpenCV matrices
        void readYamlToMatrix(const String& yamlFileName, const StringArray& featureList);
        cv::Mat pcaReduce(cv::Mat mat, int noOfDimensions);
        void normaliseFeatures(cv::Mat mat); //in-place

        CvKNearest knn;
        
        cv::Mat kMeans(cv::Mat points, int k);
        cv::Mat knnClassify(cv::Mat instance, int k);
        cv::Mat knnTrain(cv::Mat points, cv::Mat classes);
    };
}

#endif  // INFORMATION_H_INCLUDED
