//
//
// histogramPeakHeights_3316.C
//
//
// created by g.c. rich and unleashed for consumption 10-28-14
//
//
// this is intended to provide a simple example of how analysis
// might be carried out on a ROOT file containing 3316 digitized data
// it is assumed that the ROOT file is structured like those
// produced by binaryToROOT-NGM3316
//
// the function accepts two arguments, the second of which is optional:
// a string which is the filename of the ROOT file to examine
// an integer which specifies which channel's peak heights to histogram
//
//
// this file includes the bones for a more robust analysis of this data
// the algorithm in the loop over events can be rewritten to address
// many other tasks
//

Int_t histogramPeakHeights_3316( TString inputFilename, Int_t chanNum = 0 ) {
    
    // define the TFile we use to read in the data
    TFile* inputFile = new TFile( inputFilename, "READ" );
    
    // see if we opened it successfully
    // if not, we should quit, returning -1
    if( inputFile->IsOpen() == 0 ) {
        printf( "\n!!!!!!!\n" );
        printf( "Problem opening specified file %s\n", inputFile.Data() );
        printf( "Does file exist?\n" );
        printf( "Exiting...\n" );
        return -1;
    }
    
    // get the tree containing all the data from the file
    TTree* dataTree = (TTree*)inputFile->Get( "sis3316tree" );
    
    // check to see that we grabbed the tree successfully
    // if not, exit, returning -2
    if( dataTree == 0 ) {
        printf( "\n!!!!!!!\n" );
        printf( "Problem getting tree from file\n" );
        printf( "Exiting...\n" );
        return -2;
    }
    
    
    // define all the variables to store data from file
    ULong64_t timestamp;
    UShort_t peakHighIndex;
    UShort_t peakHighValue;
    UShort_t channelID;
    UChar_t formatBits;
    UInt_t accumulatorSum[8];
    UChar_t informationBits;
    UInt_t mawMaximumValue;
    UInt_t mawValueAfterTrigger;
    UInt_t mawValueBeforeTrigger;
    UInt_t mawTestData;
    UInt_t startEnergyValue;
    UInt_t maxEnergyValue;
    Bool_t mawTestFlag;
    Bool_t pileupFlag;
    UInt_t nSamples;
    UShort_t waveform[65536];
    
    // set up all the branch addresses
    // this way, the tree knows where to push data as we retrieve events
    //
    // for our example (histogramming peak values), this is excessive
    // all we really need is "peakHighValue"
    dataTree->SetBranchAddress( "channelID", &channelID );
    dataTree->SetBranchAddress( "timestamp", &timestamp );
    dataTree->SetBranchAddress( "peakHighIndex", &peakHighIndex );
    dataTree->SetBranchAddress( "peakHighValue", &peakHighValue );
    dataTree->SetBranchAddress( "formatBits", &formatBits );
    dataTree->SetBranchAddress( "accumulatorSum", accumulatorSum );
    dataTree->SetBranchAddress( "informationBits", &informationBits );
    dataTree->SetBranchAddress( "mawMaximumValue", &mawMaximumValue );
    dataTree->SetBranchAddress( "mawValueAfterTrigger",
                               &mawValueAfterTrigger );
    dataTree->SetBranchAddress( "mawValueBeforeTrigger",
                               &mawValueBeforeTrigger );
    dataTree->SetBranchAddress( "mawTestData", &mawTestData );
    dataTree->SetBranchAddress( "startEnergyValue", &startEnergyValue );
    dataTree->SetBranchAddress( "maxEnergyValue", &maxEnergyValue );
    dataTree->SetBranchAddress( "mawTestFlag", &mawTestFlag );
    dataTree->SetBranchAddress( "pileupFlag", &pileupFlag );
    dataTree->SetBranchAddress( "nSamples", &nSamples );
    dataTree->SetBranchAddress( "waveform", waveform );
    
    
    
    char strBuff[256];
    // make the histogram we're going to populate
    // peakHighValue is a 14-bit number
    // so bin it for 14 bits
    TString titleString = "Histogram of peak-height values for channel ";
    sprintf( strBuff, "%i", chanNum );
    titleString.Append( strBuff );
    titleString.Append( ";Peak height (ADC units); Counts" );
    TH1D* peakHeightHistogram = new TH1D( "peakHeightHistogram",
                                       titleString, 16384, 0, 16383 );
    
    // get the number of entries in the tree
    Long64_t nEntries = dataTree->GetEntries();
    
    printf( "Tree has %lli entries\n", nEntries );
    
    //
    // loop over all the data in the tree
    //
    for( Long64_t entryNumber = 0; entryNumber < nEntries; entryNumber++ ) {
        
        // print out updates on progress periodically for the impatient
        if( entryNumber % 100000 == 0 ) {
            printf( "Processing entry %lli\n", entryNumber );
        }
        
        // get the entry
        // this populates our variables with data from the tree
        // the data is updated each time we call GetEntry( # )
        // data corresponds to that from entry number specified
        dataTree->GetEntry( entryNumber );
        
        // populate our histogram with data from this event
        if( chanNum == channelID ) {
            peakHeightHistogram->Fill( peakHighValue );
        }
    }
    
    // make a canvas on which we'll draw our histogram
    TCanvas* histogramDisplay_canvas = new TCanvas( "displayCanvas",
                                                   "displayCanvas",
                                                   1000, 650 );
    
    // make sure this canvas is "active", so we draw on it
    histogramDisplay_canvas->cd(1);
    
    // draw (a copy of) our histogram
    // drawing a copy insulates this drawing from subsequent changes
    peakHeightHistogram->DrawCopy("");
    
    // make the y axis logarithmic
    gPad->SetLogy();
}