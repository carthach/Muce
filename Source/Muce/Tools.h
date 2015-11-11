/*
  ==============================================================================

    Audio.h
    Created: 18 Oct 2015 11:58:36am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#ifndef TOOLS_H_INCLUDED
#define TOOLS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "opencv2/opencv.hpp"

#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>

//#include "Extraction.h"

namespace Muce {
    class Tools{
    public:
        Tools()
        {
            formatManager.registerBasicFormats();
        }
        
        ~Tools() {};
        
        //Figure out where to put this stuff
        typedef std::map<std::string, std::vector<essentia::Real> > RealMap;
        typedef std::map<std::string, std::vector< std::vector<essentia::Real> > > VectorMap;
        typedef std::map<std::string, std::vector<essentia::Real> >::iterator RealMapIter;
        typedef std::map<std::string, std::vector< std::vector<essentia::Real> > >::iterator VectorMapIter;
        
        AudioFormatManager formatManager;

        static std::vector<float> hannWindow(int size);
        static Array<File> getAudioFiles(const File& audioFolder);
        static bool vectorToAudioFile(const std::vector<essentia::Real> signal, const String fileName);
        
        AudioSampleBuffer audioFileToSampleBuffer(const File audioFile);
        std::vector<essentia::Real> audioFileToVector(const File audioFile);
        
        std::vector<essentia::Real> removeLabels(essentia::Pool& pool);
        cv::Mat poolToMat(const essentia::Pool& pool);
        
        String getAppendedFilename(const File & file, String appendText);
    };
}

#endif  // AUDIO_H_INCLUDED
