/*
  ==============================================================================

    Extraction.cpp
    Created: 18 Oct 2015 11:58:03am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#include "Extraction.h"

namespace Muce {        
    Extraction::Extraction()
    {
        if(!essentia::isInitialized())
            essentia::init();
        initBaseAlgorithms();
    }
    
    Extraction::~Extraction()
    {
        essentia::shutdown();
    }
    
    void Extraction::initBaseAlgorithms()
    {
        AlgorithmFactory& factory = standard::AlgorithmFactory::instance();

        
        
        frameCutter    = factory.create("FrameCutter",
                                        "frameSize", frameSize,
                                        "hopSize", hopSize);
        window     = factory.create("Windowing", "type", "hann");
        spec  = factory.create("Spectrum");
        
        frameCutter->output("frame").set(frame);
        window->input("frame").set(frame);
        
        window->output("frame").set(windowedFrame);
        spec->input("frame").set(windowedFrame);
        
        spec->output("spectrum").set(spectrum);
    
        
        //Yaml Output in JSON format
        //Aggregate Pool stats
        string defaultStats[] = {"mean", "var"};
        poolAggregator = factory.create("PoolAggregator", "defaultStats", arrayToVector<string>(defaultStats));
        
        yamlOutput  = factory.create("YamlOutput", "format", "yaml");
    }
    
    void Extraction::setupStatistics(StringArray statistics)
    {
        selectedStatistics.clear();
        for(auto statistic : statistics)
            selectedStatistics.push_back(statistic.toStdString());
            
        AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
        
        poolAggregator = factory.create("PoolAggregator", "defaultStats", selectedStatistics);
    }
    
    void Extraction::setupUserAlgorithms(StringArray algorithmChoices)
    {
        selectedAlgorithms.clear();
        for(int i=0; i<algorithmChoices.size(); i++)
            selectedAlgorithms[algorithmChoices[i].toStdString()] = true;
        
        AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
        
        
        //Stastical things
        float halfSampleRate = (float)sampleRate / 2.0;
        
        if(selectedAlgorithms["MFCC"]) {
            algorithms["MFCC"] = factory.create("MFCC");
            algorithms["MFCC"]->input("spectrum").set(spectrum);
            algorithms["MFCC"]->output("bands").set(mfccBands);
            algorithms["MFCC"]->output("mfcc").set(mfccCoeffs);
        }
        
        if(selectedAlgorithms["Centroid"]) {
            algorithms["Centroid"] = factory.create("Centroid", "range", halfSampleRate);
            algorithms["Centroid"]->input("array").set(spectrum);
            algorithms["Centroid"]->output("centroid").set(spectralCentroid);
        }
        
        if(selectedAlgorithms["Flatness"]) {
            algorithms["Flatness"] = factory.create("FlatnessDB");
            algorithms["Flatness"]->input("array").set(spectrum);
            algorithms["Flatness"]->output("flatnessDB").set(spectralFlatnessReal);
        }
        
        if(selectedAlgorithms["Bands"]) {
            algorithms["Bands"] = factory.create("ERBBands");
            algorithms["Bands"]->input("spectrum").set(spectrum);
            algorithms["Bands"]->output("bands").set(bandsVector);
        }
        
        if(selectedAlgorithms["Pitch"]) {
            algorithms["Pitch"] = factory.create("PitchYinFFT");
            algorithms["Pitch"]->input("spectrum").set(spectrum);
            algorithms["Pitch"]->output("pitch").set(pitchReal);
            algorithms["Pitch"]->output("pitchConfidence").set(pitchConfidence);
            algorithms["Pitch"]->output("pitch").set(pitchReal);
        }
        
        //Global
        if(selectedAlgorithms["RMS"]) {
            algorithms["RMS"] = factory.create("RMS");
            algorithms["RMS"]->output("rms").set(rmsReal);
        }
        
        if(selectedAlgorithms["ZeroCrossingRate"]) {
            algorithms["ZeroCrossingRate"] = factory.create("ZeroCrossingRate");
            algorithms["ZeroCrossingRate"]->output("ZeroCrossingRate").set(zeroCrossingRateReal);
        }
        
        if(selectedAlgorithms["LogAttackTime"]) {
            algorithms["LogAttackTime"] = factory.create("LogAttackTime");
            algorithms["LogAttackTime"]->output("logAttackTime").set(logAttackTimeReal);
        }
        
        if(selectedAlgorithms["Envelope"])
        {
            algorithms["Envelope"] = factory.create("Envelope");
            algorithms["Envelope"]->output("signal").set(envelopeSignal);
        }
        
        if(algorithms.count("Envelope") && selectedAlgorithms["TcToTotal"]) {
            algorithms["TcToTotal"] = factory.create("TCToTotal");
            algorithms["TcToTotal"]->input("envelope").set(envelopeSignal);
            algorithms["TcToTotal"]->output("TCToTotal").set(tCToTotalReal);
        }
    }
    
    
    vector<Real> Extraction::extractPeakValues(const vector<vector<Real> >& slices)
    {
        vector<Real> peakValues(slices.size(), 0.0f);
        for(int i=0; i<slices.size(); i++) {
            for(int j=0; j<slices[i].size(); j++) {
                float absSample = fabs(slices[i][j]);
                if (absSample > peakValues[i]) {
                    peakValues[i] = absSample;
                }
            }
        }
        return peakValues;
    }
    
    vector<Real> Extraction::extractOnsetTimes(const vector<Real>& audio)
    {
        ScopedPointer<Algorithm> extractoronsetrate = AlgorithmFactory::create("OnsetRate");
        
        Real onsetRate;
        vector<Real> onsets;
        
        extractoronsetrate->input("signal").set(audio);
        extractoronsetrate->output("onsets").set(onsets);
        //    extractoronsetrate->configure("ratioThreshold", 12.0);
        //    extractoronsetrate->configure("combine", 100.0);
        extractoronsetrate->output("onsetRate").set(onsetRate);
        
        extractoronsetrate->compute();
                
        return onsets;
    }
    
    
    
    vector<Real> Extraction::extractRhythmFeatures(const vector<Real>& audio)
    {
        ScopedPointer<Algorithm> rhythmExtractor = AlgorithmFactory::create("RhythmExtractor2013", "method", "degara");
        
        Real bpm ,confidence;
        vector<Real> ticks, estimates, bpmIntervals;
        
        rhythmExtractor->input("signal").set(audio);
        rhythmExtractor->output("bpm").set(bpm);
        rhythmExtractor->output("ticks").set(ticks);
        rhythmExtractor->output("estimates").set(estimates);
        rhythmExtractor->output("bpmIntervals").set(bpmIntervals);
        rhythmExtractor->output("confidence").set(confidence);
        
        rhythmExtractor->compute();
        
        vector<Real> blah;
        blah.push_back(bpm);
        
        return blah;
    }
    
    //vector<Real> Extraction::getGlobalFeatures(const vector<Real>& audio)
    //{
    //    Algorithm* bpmHistogram = AlgorithmFactory::create("RhythmDescriptors");
    //
    //    Real bpm;
    //
    //    vector<Real> vec;
    //
    //    bpmHistogram->input("signal").set(audio);
    //    bpmHistogram->output("bpm").set(bpm);
    //    bpmHistogram->output("beats_position").set("");
    //
    //    bpmHistogram->compute();
    //
    //    vec.push_back(bpm);
    //
    //
    //    delete bpmHistogram;
    //
    //    return vec;
    //}
    
    std::vector<Real> Extraction::poolToVector(const Pool& pool)
    {
        std::vector<Real> featureVector;
        
        RealMap realMap = pool.getRealPool();
        VectorMap vectorMap = pool.getVectorRealPool();
        
        for(auto & realFeature : realMap) //Spectral Centroid, RMS etc.
        {
            featureVector.push_back(realFeature.second[0]);
        }
        
        for(auto & realFeatures : vectorMap) //MFCC, BANDS
        {
            for(auto & realFeature : realFeatures.second[0])
                featureVector.push_back(realFeature);
        }
        
        return featureVector;
    }
    
    vector<vector<Real> > Extraction::extractOnsets(const vector<Real>& onsetTimes, const vector<Real>& audio)
    {
        vector<vector<Real> > slices;
        AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
        
        //Fix magic SR
        float audioLength = (float)audio.size() / (float)44100.0;
        
        vector<Real>::const_iterator first = onsetTimes.begin()+1;
        vector<Real>::const_iterator last = onsetTimes.end();
        vector<Real> endTimes(first, last);
        endTimes.push_back(audioLength);
        
        Algorithm* slicer = factory.create("Slicer",
                                           "endTimes", endTimes,
                                           "startTimes",
                                           onsetTimes);
        
        slicer->input("audio").set(audio);
        slicer->output("frame").set(slices);
        slicer->compute();
        
        delete slicer;
        
        for(int i=0; i<slices.size();i++) {
            std::vector<float> hann = Tools::hannWindow(slices[i].size());
            for(int j=0; j<slices[i].size(); j++) {
                if(j <= 256)
                    slices[i][j] = slices[i][j] * hann[j];
                if(j >= (float)slices[i].size() / 4.0)
                    slices[i][j] = slices[i][j] * hann[j];
            }
        }
        
        return slices;
    }
    
    inline void Extraction::writeOnsets(const vector<vector<Real> >& slices, const String outputRoot)
    {
        
        for(int i=0; i<slices.size(); i++)
        {
            String fileName = outputRoot + "slice_" + std::to_string(sliceID++) + ".wav";
            Tools::vectorToAudioFile(slices[i], fileName);
        }
    }
    
    Pool Extraction::loadFeatures(const String& jsonFilename)
    {
        Pool pool;
        
        //We get the overall pool, merge and output
        ScopedPointer<Algorithm> yamlInput  = AlgorithmFactory::create("YamlInput", "format", "json");
        yamlInput->configure("filename", jsonFilename.toStdString());
        yamlInput->output("pool").set(pool);
        yamlInput->compute();

//        std::cout << pool.descriptorNames() << "\n";
        
//        this->globalOnsetPool = pool;
        
        delete yamlInput;
        
        return pool;
    }
    
    
    void Extraction::writeLoop(float onsetTime, const vector<Real>& audio, float BPM, String outFileName)
    {
        float startTimeInSamples = onsetTime * 44100.0;
        
        float lengthOfBeatInSamples = (1.0 / BPM) * 60.0 * 44100.0;
        
        float endTimeInSamples = startTimeInSamples + (lengthOfBeatInSamples * 8.0);
        
        while(endTimeInSamples >= (audio.size() - endTimeInSamples))
            endTimeInSamples = startTimeInSamples + (lengthOfBeatInSamples * 8.0);
        
        vector<Real>::const_iterator first = audio.begin() + (int)startTimeInSamples;
        vector<Real>::const_iterator last = audio.begin() + (int)endTimeInSamples;
        
        vector<Real> newVec(first, last);
        
        Tools::vectorToAudioFile(newVec, outFileName);
    }
    
    vector<Real> Extraction::firstLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename)
    {
        float lengthOfBeatInSamples = (1.0 / BPM) * 60.0 * 44100.0;
        
        vector<Real> newVec;
        
        int randomOnsetIndex;
        float startTimeInSamples;
        float endTimeInSamples;
        
        startTimeInSamples = onsetTimes[0] * 44100.0;
        endTimeInSamples = startTimeInSamples + (lengthOfBeatInSamples * 8.0);
        
        //Check yer out of bounds here
        
        if(endTimeInSamples <= audio.size()) {
            vector<Real>::const_iterator first = audio.begin() + (int)startTimeInSamples;
            vector<Real>::const_iterator last = audio.begin() + (int)endTimeInSamples;
            
            newVec = vector<Real>(first, last);
            
            Tools::vectorToAudioFile(newVec, outFilename);
        }
        
        return newVec;
    }
    
    vector<Real> Extraction::randomLoop(const vector<Real>& onsetTimes, const vector<Real>& audio, Real BPM, String outFilename)
    {
        float lengthOfBeatInSamples = (1.0 / BPM) * 60.0 * 44100.0;
        
        vector<Real> newVec;
        
        int randomOnsetIndex;
        float startTimeInSamples;
        float endTimeInSamples;
        
        do  {
            int randomOnsetIndex = random.nextInt(onsetTimes.size());
            startTimeInSamples = onsetTimes[randomOnsetIndex] * 44100.0;
            endTimeInSamples = startTimeInSamples + (lengthOfBeatInSamples * 8.0);
            
        } while(endTimeInSamples > (audio.size()-endTimeInSamples));
        
        vector<Real>::const_iterator first = audio.begin() + (int)startTimeInSamples;
        vector<Real>::const_iterator last = audio.begin() + (int)endTimeInSamples;
        
        newVec = vector<Real>(first, last);
        
        Tools::vectorToAudioFile(newVec, outFilename);
        
        return newVec;
    }
    
    

    
    StringArray Extraction::featuresInPool(const Pool& pool)
    {
        StringArray featureList;
        
        RealMap realMap = pool.getRealPool();
        VectorMap vectorMap = pool.getVectorRealPool();
        
        for(RealMapIter iterator = realMap.begin(); iterator != realMap.end(); iterator++)
            featureList.add(iterator->first);
        
        for(VectorMapIter iterator = vectorMap.begin(); iterator != vectorMap.end(); iterator++) {
            for(int i=0; i<iterator->second[0].size(); i++)
                featureList.add(iterator->first + "_" + String(i));
        }
        
        return featureList;
    }
    
    Pool Extraction::extractFeatures(const vector<Real>& audio)
    {
        Pool framePool, aggrPool;
        
        //Reset the framecutter and set to the new input
        frameCutter->reset();
        frameCutter->input("signal").set(audio);
        
        //Reset the framewise algorithms
        window->reset();
        spec->reset();
        
        for(auto & algorithm : algorithms)
            algorithm.second->reset();
        
        poolAggregator->reset();
        poolAggregator->input("input").set(framePool);
        poolAggregator->output("output").set(aggrPool);
        
        framePool.clear();
        
        //Start the frame cutter
        while (true) {
            
            // compute a frame
            frameCutter->compute();
            
            // if it was the last one (ie: it was empty), then we're done.
            if (!frame.size()) {
                break;
            }
            
            // if the frame is silent, just drop it and go on processing
            if (isSilent(frame)) continue;
            
            //Spectrum and MFCC
            window->compute();
            spec->compute();
            
            if(algorithms.count("MFCC")) {
                algorithms["MFCC"]->compute();
                framePool.add("mfcc",mfccCoeffs);
            }

            if(algorithms.count("Centroid")) {
                algorithms["Centroid"]->compute();
                framePool.add("centroid", spectralCentroid);
            }
            
            if(algorithms.count("Flatness")) {
                algorithms["Flatness"]->compute();
                framePool.add("flatness", spectralFlatnessReal);
            }
            
            if(algorithms.count("Bands")) {
                algorithms["Bands"]->compute();
                framePool.add("bands", bandsVector);
            }
            
            if(algorithms.count("Pitch")) {
                algorithms["Pitch"]->compute();
                framePool.add("pitch", pitchReal);
            }
            
            //            framePool.add("spectral_spread", spread);
            //            framePool.add("spectral_skewness", skewness);
            //            framePool.add("spectral_kurtosis", kurtosis);
        }
        
        //Time to aggregate
        aggrPool.clear();
        poolAggregator->reset();
        poolAggregator->compute();
        
        //For merging pools together the JSON entries need to be vectorised (using .add function)
        //aggrPool gets rid of this, so a fix is to remove and add again
        
        //Reals
        std::map< std::string, Real>  reals = aggrPool.getSingleRealPool();
        typedef std::map<std::string, Real>::iterator realsIterator;
        
        for(realsIterator iterator = reals.begin(); iterator != reals.end(); iterator++) {
            aggrPool.remove(iterator->first);
            aggrPool.add(iterator->first, iterator->second);
        }
        
        if(algorithms.count("Bands")) {
            algorithms["Bands"]->compute();
            framePool.add("bands", bandsVector);
            
            if(aggrPool.contains<vector<Real> >("bands.mean")) {
                vector<Real> aggrBands = aggrPool.value<vector<Real> >("bands.mean");
                
                ScopedPointer<Algorithm> mean = AlgorithmFactory::create("Mean");
                
                //=========== ERB STUFF =========
                //Get erbLo
                vector<Real>::const_iterator first = aggrBands.begin();
                vector<Real>::const_iterator last = aggrBands.begin() + 2;
                vector<Real> loBands(first, last);
                
                Real loValue;
                mean->input("array").set(loBands);
                mean->output("mean").set(loValue);
                mean->compute();
                aggrPool.add("loValue", loValue);
                
                //Get erbMid
                first = aggrBands.begin()+2;
                last = aggrBands.begin()+5;
                vector<Real> midBands(first, last);
                
                Real midValue;
                
                mean->reset();
                mean->input("array").set(midBands);
                mean->output("mean").set(midValue);
                mean->compute();
                aggrPool.add("midValue", midValue);
                
                //Get erbHi
                first = aggrBands.begin()+5;
                //        last = aggrBands.end();
                last = aggrBands.begin()+10;
                vector<Real> hiBands(first, last);
                
                Real hiValue;
                
                mean->reset();
                mean->input("array").set(hiBands);
                mean->output("mean").set(hiValue);
                mean->compute();
                aggrPool.add("hiValue", hiValue);
                
                //Remove the original full erbBand vectors
                aggrPool.remove("bands.mean");
                aggrPool.remove("bands.var");
            }
        }
    
        if(algorithms.count("MFCC")) {
                //Remove/Add mfcc vector
            for(auto statistic : selectedStatistics)
            {
                string key = "mfcc." + statistic;
                vector<Real> stat = aggrPool.value<vector<Real> >(key);
                aggrPool.remove(key);
                aggrPool.add(key, stat);
            }
        }
        
        if(algorithms.count("RMS")){
            algorithms["RMS"]->reset();
            algorithms["RMS"]->input("array").set(audio);
            algorithms["RMS"]->compute();
            aggrPool.add("RMS", rmsReal);
        }
        
        if(algorithms.count("RMS")) {
            
        }
        if(algorithms.count("ZeroCrossingRate")) {
            algorithms["ZeroCrossingRate"]->reset();
            algorithms["ZeroCrossingRate"]->input("signal").set(audio);
            algorithms["ZeroCrossingRate"]->compute();
            aggrPool.add("ZeroCrossingRate", zeroCrossingRateReal);
        }
        
        if(algorithms.count("LogAttackTime")) {
            algorithms["LogAttackTime"]->reset();
            algorithms["LogAttackTime"]->input("signal").set(audio);
            algorithms["LogAttackTime"]->compute();
            aggrPool.add("LogAttackTime", logAttackTimeReal);
        }
        
        if(algorithms.count("Envelope")) {
            algorithms["Envelope"]->reset();
            algorithms["Envelope"]->input("signal").set(audio);
            algorithms["Envelope"]->compute();
            aggrPool.add("Envelope", envelopeSignal);
        }
        
        if(algorithms.count("TCToTotal")) {
            algorithms["TcToTotal"]->reset();
            algorithms["TcToTotal"]->compute();
            aggrPool.add("TCToToal", tCToTotalReal);
        }
                
        //If you want to output individual aggregate pools
        //            if(outputAggrPool) {
        //                yamlOutput->reset();
        //                yamlOutput->input("pool").set(aggrPool);
        //                string fileName = "slice_" + std::to_string(localSliceID++) + ".yaml";
        //                yamlOutput->configure("filename", fileName);
        //                yamlOutput->compute();
        //            }
        
        //Finally do the merge to the overall onsetPool
        
        return aggrPool;
    }
    
    //Put your extractor code here
    Pool Extraction::extractFeaturesFromOnsets(vector<vector<Real> >& slices)
    {
        Pool onsetPool;
        
        for(auto slice : slices)
        {
            Pool onsetFeatures = extractFeatures(slice);
            onsetPool.merge(onsetFeatures, "append");
        }
        
        return onsetPool;
    }
}