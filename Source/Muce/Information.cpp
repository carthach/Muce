/*
  ==============================================================================

    Information.cpp
    Created: 18 Oct 2015 11:58:19am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#include "Information.h"


namespace Muce {
    Information::Information()
    {
        
    }
    Information::~Information()
    {
        
    }
    
    void Information::scaleFeatures(cv::Mat mat, std::vector<float> weights)
    {
        for(int i=0; i<mat.rows;i++)
            for(int j=0;j<mat.cols; j++)
                mat.at<float>(i, j) *= weights[j];
    }
    
    void Information::normaliseFeaturesWithTarget(cv::Mat & targetMat, cv::Mat & datasetMat)
    {
        //Merge matrices
        datasetMat.push_back(targetMat);
        
        //Normalise
        normaliseFeatures(datasetMat);
        
        //Delace
        targetMat = datasetMat.rowRange(datasetMat.rows-targetMat.rows, datasetMat.rows);
                
        for(int i=0 ;i<targetMat.rows; i++)
            datasetMat.pop_back();
        
    }
    
    void Information::normaliseFeaturesWithTarget(cv::Mat & targetMat, cv::Mat & datasetMat, std::vector<float> & weights)
    {
        //Merge matrices
        datasetMat.push_back(targetMat);
        
        //Normalise
        normaliseFeatures(datasetMat);
        
        //Delace
        targetMat = datasetMat.rowRange(datasetMat.rows-datasetMat.rows, datasetMat.rows);
        
        for(int i=0 ;i<targetMat.rows; i++)
            datasetMat.pop_back();
    }
    
//    void Information::normaliseTargetAndDataset(cv::Mat targetMat, cv::Mat datasetMat)
//    {
//        datasetMat.push_back(targetMat);
//        normaliseFeatures(datasetMat);
//        datasetMat.pop_back()
//    }
    
    
//    cv::Mat Information::knnTrain(essentia::Pool pool)
//    {
//        //Train Classifier
//        if (pool.contains<std::vector<essentia::Real> >("labels")) {
//            std::vector<essentia::Real> labelsVector = pool.value<std::vector<essentia::Real> >("labels");
//            pool.remove("labels");
//            
//            cv::Mat features = poolToMat(pool);
//            cv::Mat labels(labelsVector, true);
//            
//            knn.train(features, labels);
//        }        
//    }
        
    cv::Mat Information::kMeans(cv::Mat points, int k)
    {
        using namespace cv;
        cv::Mat labels, centers;
        
        kmeans(points, k, labels,
               TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 10, 1.0),
               3, KMEANS_PP_CENTERS, centers);
        
        return labels;
    }
    
    cv::Mat Information::knnClassify(cv::Mat instances, int k)
    {
        cv::Mat results;
        knn.find_nearest(instances, k, &results, 0, 0, 0);
        
        return results;
    }
    
    /* Use openCV and PCA to collapse the MFCCs to 2D points for visualisation */
    
    cv::Mat Information::pcaReduce(cv::Mat mat, int noOfDimensions)
    {
        cv::Mat projection_result;
        
        cv::PCA pca(mat,cv::Mat(),CV_PCA_DATA_AS_ROW, noOfDimensions);
        
        pca.project(mat,projection_result);
        
        return projection_result;
    }
    
    std::vector<std::vector<float> > Information::matToVectors(cv::Mat mat)
    {
        std::vector<std::vector<float> > vectors;
        
        for(int i=0; i<mat.rows; i++) {
            vectors.push_back(rowToVector(mat.row(i)));
        }
        return vectors;
    }
    
    std::vector<float> Information::rowToVector(cv::Mat row)
    {
        // Pointer to the i-th row
//        jassert(row.rows == 0);
        
        const float* p = row.ptr<float>(0);
        
        // Copy data to a vector.  Note that (p + mat.cols) points to the
        // end of the row.
        std::vector<float> vec(p, p + row.cols);
        return vec;
    }
    
    /* Use openCV and PCA to collapse the MFCCs to 2D points for visualisation */
    
    void Information::normaliseFeatures(cv::Mat mat)
    {
        for(int i=0; i <mat.cols; i++) {
            cv::normalize(mat.col(i), mat.col(i), 0, 1, cv::NORM_MINMAX, CV_32F);
        }
    }
    
    cv::Mat Information::getDistanceMatrix(cv::Mat targetMatrix, cv::Mat datasetMatrix)
    {
        cv::Mat distanceMatrix(datasetMatrix.rows, targetMatrix.rows, cv::DataType<float>::type);
        
        for(int i=0; i<targetMatrix.rows; i++) { //No of onsets
            for(int j=0; j<datasetMatrix.rows; j++) { //No of dataset slices
                
                float dist = norm(targetMatrix.row(i), datasetMatrix.row(j));
                
                distanceMatrix.at<float>(j,i) = dist;
            }
        }
        return distanceMatrix;
    }
    
    cv::Mat Information::getSimilarityMatrix(cv::Mat distanceMatrix)
    {
        cv::Mat similarityMatrix;
        cv::sortIdx(distanceMatrix, similarityMatrix, cv::SORT_EVERY_COLUMN + cv::SORT_ASCENDING);
        return similarityMatrix;
    }
    
    void Information::readYamlToMatrix(const String& yamlFilename, const StringArray& featureList)
    {
//        using namespace cv;
//        
//        FileStorage::FileStorage fs2(yamlFilename.toStdString(), FileStorage::READ);
//        
//        FileNode features = fs2["erbHi"];
//        
//        std::cout << (float)features[0];
//        
//        
//        //    // first method: use (type) operator on FileNode.
//        //    int frameCount = (int)fs2["frameCount"];
//        //
//        //    std::string date;
//        //    // second method: use FileNode::operator >>
//        //    fs2["calibrationDate"] >> date;
//        //
//        //    Mat cameraMatrix2, distCoeffs2;
//        //    fs2["cameraMatrix"] >> cameraMatrix2;
//        //    fs2["distCoeffs"] >> distCoeffs2;
//        //
//        //    cout << "frameCount: " << frameCount << endl
//        //    << "calibration date: " << date << endl
//        //    << "camera matrix: " << cameraMatrix2 << endl
//        //    << "distortion coeffs: " << distCoeffs2 << endl;
//        //
//        //    FileNode features = fs2["features"];
//        //    FileNodeIterator it = features.begin(), it_end = features.end();
//        //    int idx = 0;
//        //    std::vector<uchar> lbpval;
//        //    
//        //    // iterate through a sequence using FileNodeIterator
//        //    for( ; it != it_end; ++it, idx++ )
//        //    {
//        //        cout << "feature #" << idx << ": ";
//        //        cout << "x=" << (int)(*it)["x"] << ", y=" << (int)(*it)["y"] << ", lbp: (";
//        //        // you can also easily read numerical arrays using FileNode >> std::vector operator.
//        //        (*it)["lbp"] >> lbpval;
//        //        for( int i = 0; i < (int)lbpval.size(); i++ )
//        //            cout << " " << (int)lbpval[i];
//        //        cout << ")" << endl;
//        //    }
//        
//        fs2.release();
    }
    
    void clusterData(cv::Mat data)
    {
        cv::Mat labels;
        cv::kmeans(data, 3, labels, cv::TermCriteria( cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 10, 1.0), 3, cv::KMEANS_PP_CENTERS, cv::noArray());
    }
}