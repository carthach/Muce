//
//  Batch.cpp
//  rhythmCAT
//
//  Created by Cárthach Ó Nuanáin on 11/11/2015.
//
//

#include <stdio.h>

#include "../JuceLibraryCode/JuceHeader.h"
#include "Extraction.h"
#include "Tools.h"

using namespace Muce;

//  This is an example of using a Juce Thread with progress window to compute all features
//  in a batch of files and its subsequent onsets, adapt accordingly

class ThreadBatch : public ThreadWithProgressWindow {
public:
    Extraction& extraction;
    Tools& tools;
    
    File directory;
    bool writeOnsets = false;
    Pool pool;
    
    ThreadBatch(Extraction& extractionIn) : ThreadWithProgressWindow ("Building Dataset...", true, true), extraction(extractionIn), tools(extraction.tools)
    {

    }
    
    void run()
    {
        using namespace essentia;
        using namespace essentia::standard;
        
        StringArray algorithms = {"MFCC", "Centroid", "Flatness"};
        extraction.setupUserAlgorithms(algorithms);
        
        Array<File> audioFiles = tools.getAudioFiles(directory);
        
        File datasetRoot(directory.getFullPathName() + "/dataset/");
        if(datasetRoot.exists())
            datasetRoot.deleteRecursively();
        datasetRoot.createDirectory();
        
        int fileCounter = 0;
        int onsetCounter = 0;
        
        for(fileCounter=0; fileCounter<audioFiles.size(); fileCounter++)
        {
            // must check this as often as possible, because this is
            // how we know if the user's pressed 'cancel'
            if (threadShouldExit())
                break;
            
            //Input audio
            vector<Real> audio =  tools.audioFileToVector(audioFiles[fileCounter]);
            
            //Get slices
            vector<Real> onsetTimes = extraction.extractOnsetTimes(audio);
            
            //Then get the audio
            vector<vector<Real> > audioOnsets = extraction.extractOnsets(onsetTimes, audio);
            
            //Then go through the onsets and extract the features
            for(onsetCounter=0; onsetCounter < audioOnsets.size();onsetCounter++)
            {
                //Extract Features
                Pool onsetPool = extraction.extractFeatures(audioOnsets[onsetCounter], 120.0);
                
                //Write Onsets
                if(writeOnsets) {
                    String audioSliceFilename = datasetRoot.getFullPathName() + "/" + tools.getAppendedFilename(audioFiles[fileCounter], "_slice_" + String(onsetCounter));
                    File audioSliceFile = File(audioSliceFilename);
                    tools.vectorToAudioFile(audioOnsets[onsetCounter], audioSliceFilename);
                }
                
                //Merge to the main pool
                pool.merge(onsetPool, "append");
            }
            
            //Update progress
            setProgress (fileCounter / (double) audioFiles.size());            
        }
    }
    
    //This does the extraction
    Pool batchExtract(const File & directory, bool writeOnsets)
    {
        this->directory = directory;
        this->writeOnsets = writeOnsets;
        
        pool.clear();
        
        if(runThread())
            ;
        else
            ; //Cancelled

        return pool;
    }
    
};