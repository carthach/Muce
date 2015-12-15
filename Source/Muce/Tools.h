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
        
        //Calculates and returns length of a sample of various note lengths
        double getPeriodOfNoteInSamples(double BPM, double sampleRate, double noteLength)
        {
            double periodOfQuaver = (60.0 * sampleRate) / BPM;
            
            if(noteLength > 4)
                periodOfQuaver = periodOfQuaver / (noteLength / 4);
            else if(noteLength == 2)
                periodOfQuaver *= 2;
            else if(noteLength == 1)
                periodOfQuaver *= 4;
            
            return periodOfQuaver;
        }
        
        //================================================
        
        AudioFormatManager formatManager;
        
        //Figure out where to put this stuff
        typedef std::map<std::string, std::vector<essentia::Real> > RealMap;
        typedef std::map<std::string, std::vector< std::vector<essentia::Real> > > VectorMap;
        typedef std::map<std::string, std::vector<essentia::Real> >::iterator RealMapIter;
        typedef std::map<std::string, std::vector< std::vector<essentia::Real> > >::iterator VectorMapIter;
    
        //Audio handling
        static std::vector<float> hannWindow(int size);
        static Array<File> getAudioFiles(const File& audioFolder);
        static bool vectorToAudioFile(const std::vector<essentia::Real> signal, const String fileName);
        
        AudioSampleBuffer audioFileToSampleBuffer(const File audioFile);
        std::vector<essentia::Real> audioFileToVector(const File audioFile);
        
        //Conversion between Extraction and Information
        std::vector<essentia::Real> removeLabels(essentia::Pool& pool);
        static cv::Mat poolToMat(const essentia::Pool& pool);
        
        //Chop off a string and append some text then add the extension again
        String getAppendedFilename(const File & file, String appendText);
        
        
        Array<File> getAudioFiles(const Array<File> & fileArray)
        {
            WildcardFileFilter wildCardFileFilter("*.wav;*.aiff;*.mp3", "", "Audio Files");
            
            Array<File> audioFiles;
            
            for(auto & file : fileArray)
                if(wildCardFileFilter.isFileSuitable(file))
                    audioFiles.add(file);
            
            return audioFiles;
        }
        
        Array<File> getAudioFiles(const StringArray & filenameArray)
        {
            Array<File> files;
            
            for(auto & filename : filenameArray)
                files.add(File(filename));
            
            return getAudioFiles(files);
        }
    };
}

#endif  // AUDIO_H_INCLUDED
