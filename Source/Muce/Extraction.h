/*
  ==============================================================================

    Extraction.h
    Created: 18 Oct 2015 11:58:03am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#ifndef EXTRACTION_H_INCLUDED
#define EXTRACTION_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/pool.h>

#include "Tools.h"

namespace Muce {
    using namespace std;
    using namespace essentia;
    using namespace essentia::standard;
    
    //Feature map typedefs
    typedef std::map<std::string, std::vector<Real> > RealMap;
    typedef std::map<std::string, std::vector< std::vector<Real> > > VectorMap;
    typedef std::map<std::string, std::vector<Real> >::iterator RealMapIter;
    typedef std::map<std::string, std::vector< std::vector<Real> > >::iterator VectorMapIter;
    
    class Extraction : public ThreadWithProgressWindow {
    public:
        int sliceID;
        Random random;
        
        Extraction();
        ~Extraction();
        void initBaseAlgorithms();
        void setupUserAlgorithms(StringArray algorithms);
        void setupStatistics(StringArray statistics);
        
        int sampleRate = 44100;
        int frameSize = 2048;
        int hopSize = 1024;
        
        //Declare global algorithms
        void poolToJson(const Pool & pool);
        
        //Handling onsets
        vector<Real> extractOnsetTimes(const vector<Real>& audio);
        vector<vector<Real> > extractOnsets(const vector<Real>& onsetTimes, const vector<Real>& audio);
        vector<vector<Real> > extractOnsets(const vector<Real>& onsetTimes, const vector<Real>& audio, int numOnsets);
        
        vector<Real> extractPeakValues(const vector<vector<Real> >& slices);
        void writeOnsets(const vector<vector<Real> >& slices, const String outputRoot);

        //Load Features
        Pool loadFeatures(const String& jsonFileName);
        
        //Global rhythm Features extractor
        vector<Real> extractRhythmFeatures(const vector<Real>& audio);
        
        //Our threaded parameters and method
        File threadAudioFolder;
        bool threadWriteOnsets;
        Pool threadFolderPool;
        void run() override;
        
        double getPeriodOfNoteInSamples(double BPM, double sampleRate, double noteLength)
        {
            double divisor = BPM * sampleRate;
            
            double periodOfQuaver = 60.0 / divisor;
            
            if(noteLength > 4)
                periodOfQuaver = periodOfQuaver / (noteLength / 4);
            if(noteLength == 2)
                periodOfQuaver *= 2;
            else
                periodOfQuaver *= 4;
            
            return periodOfQuaver;
        }
        
        //The three possible levels of feature extraction, each returning pools
        Pool extractFeaturesFromFolder(const File& audioFolder, bool writeOnsets);
        Pool extractFeaturesFromOnsets(vector<vector<Real> >& slices);
        Pool extractFeatures(const vector<Real>& audio);
        
        //Music Hack Day functions for writing loops
        void writeLoop(float onsetTime, const vector<Real>& audio, float BPM, String outFileName);
        vector<Real> randomLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename);
        vector<Real> firstLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename);
        
        StringArray featuresInPool(const Pool& pool);
        
        Tools tools;
        
        std::vector<Real> poolToVector(const Pool& pool);
        
        StringArray availableAlgorithms = {"MFCC", "Centroid", "Flatness", "Bands", "Pitch",
            "RMS", "ZeroCrossingRate", "LogAttackTime", "Envelope", "TcToTotal"};
        std::map<string, bool> selectedAlgorithms;
        std::vector<string> selectedStatistics;
    private:
        //Spectral
        ScopedPointer<Algorithm> frameCutter, window, spec;
        std::map<string, ScopedPointer<Algorithm>> algorithms;
        
        ScopedPointer<Algorithm> yamlOutput, poolAggregator;        
        
        // FrameCutter -> Windowing -> Spectrum
        std::vector<Real> frame, windowedFrame;
        
        // Spectrum -> MFCC
        std::vector<Real> spectrum, mfccCoeffs, mfccBands;
        
        // Bands
        vector<Real> bandsVector;
        
        //Spectral Centroid
        Real spectralCentroid;
        
        //MHD descriptors
        Real pitchReal, pitchConfidence;
        
        Real spectralFlatnessReal;
        
        vector<Real> moments;
        
        Real spread, skewness, kurtosis;
        
        Real rmsReal;
        Real zeroCrossingRateReal;
        Real logAttackTimeReal;
        vector<Real> envelopeSignal;
        Real tCToTotalReal;
    };
    

}

#endif  // EXTRACTION_H_INCLUDED
