/*
  ==============================================================================

    Information.h
    Created: 18 Oct 2015 11:58:19am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#ifndef INFORMATION_H_INCLUDED
#define INFORMATION_H_INCLUDED

#include "opencv2/opencv.hpp"

#include "Extraction.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>



namespace Muce {
    class Information {
    public:
        Information();
        ~Information();
        
        //Conversion to OpenCV matrices
        void readYamlToMatrix(const String& yamlFileName, const StringArray& featureList);
        cv::Mat pcaReduce(cv::Mat mat, int noOfDimensions);
        void normaliseFeatures(cv::Mat mat); //in-place
        void scaleFeatures(cv::Mat mat, std::vector<float> weights);
        
        void normaliseFeaturesWithTarget(cv::Mat & targetMat, cv::Mat &datasetMat, std::vector<float> & weights);
        void normaliseFeaturesWithTarget(cv::Mat & targetMat, cv::Mat & datasetMat);
        
        std::vector<std::vector<float> > matToVectors(cv::Mat mat);
        std::vector<float> rowToVector(cv::Mat row);

        CvKNearest knn;
        
        cv::Mat kMeans(cv::Mat points, int k);
        cv::Mat knnClassify(cv::Mat instance, int k);
        cv::Mat knnTrain(cv::Mat points, cv::Mat classes);
        
        cv::Mat getDistanceMatrix(cv::Mat targetMatrix, cv::Mat datasetMatrix);
        cv::Mat getSimilarityMatrix(cv::Mat distanceMatrix);        
    };
}

#endif  // INFORMATION_H_INCLUDED
