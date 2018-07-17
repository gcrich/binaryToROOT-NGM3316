//
//  binaryConverter-NGM3316.cc
//  
//
//  Created by grayson rich on 10/6/14.
//
//
//
//  modeled loosely after code by K. Kazkaz (LLNL)
//  original decoder by Kazkaz was used for 3320-generated data
//
//  this converter is meant to translate an NGM-binary file into a ROOT version
//  no analysis is performed, no data is removed except for NGM headers
//
#define DEBUG 0


//#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cstdlib>

#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TTreeIndex.h"
#include "TTemplWaveform.hh"
#include "SystemOfUnits.hh"

using namespace std;


//
//
// ASSUMING SAMPLING FREQUENCY OF 250 MS/s
// ASSUMING WAVEFORMS SHORTER THAN 65536 SAMPLES
//
//



// global buffer for waveforms
// so we don't have to reallocate it every time
Short_t* waveformBuffer;


// the file pointer supplied should already have the location pointer set correctly..
TTemplWaveform<Short_t>* getWaveformForChannel( ifstream* inFile, UInt_t* packetWords, UInt_t nSamples ) {
    
    UInt_t tmpWord;
    
    for( Int_t i = 0; i < (nSamples / 2); i++ ) {
        inFile->read( (char*)&tmpWord, 4 );
        (*packetWords) -= 1;
        waveformBuffer[i*2] = (Short_t)(tmpWord & 0xffff);
        waveformBuffer[i*2 + 1] = (Short_t)((tmpWord & 0xffff0000) >> 16);
    }
    
    TTemplWaveform<Short_t>* wave = new TTemplWaveform<Short_t>( waveformBuffer, nSamples );
    wave->SetSamplingFreq( 250 * CLHEP::megahertz );
    
    if(!wave) {
        printf("problem making wave!\n");
    }
    return wave;
}




Int_t main( Int_t argc, char** argv ) {
    
    //
	//	Print out usage information
	//
	if( argc != 2 ) {
        printf( "\nbinaryConverter-NGM3316\n");
        printf( "Usage: binaryConverter-NGM3316 <input file name>\n" );
		exit(0);
	}
    
    UShort_t numCards=1;
    UShort_t channelsPerCard=16;
    
    waveformBuffer = new Short_t[65536];
    
    TString inputFilename(argv[1]);
    
    
    // check to see if the input file ends in '.bin' extension
    // if it doesn't, exit the program - user probably entered something wrong
    if( inputFilename( inputFilename.Last('.') + 1, inputFilename.Length() ) != "bin" ) {
        printf("\n!!!!!\n");
        printf("Input filename provided: %s\n", inputFilename.Data() );
        printf("Input filename should have .bin extension!\n" );
        printf("Exiting..\n");
        printf("!!!!!\n");
        exit(-1);
    }
    // generate the name for the output file
    // same as input file, but replace "bin" with "root"
    TString outfileName( inputFilename );
    outfileName.Replace( inputFilename.Last('.') + 1, 3, "root" );
    
    
    TString baseName( inputFilename( 0, inputFilename.Last('.' ) ) );
    
    
    
    
    ifstream inFile;
    
    

    //
	//	Create the output file and TTree
	//
    
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
//    UShort_t waveform[65536];
    TTemplWaveform<Short_t>* wave;
    TTemplWaveform<Short_t>* sortingWave = new TTemplWaveform<Short_t>();
    
    UInt_t readBuffer[4096];
    char* bufferPointer = (char*)readBuffer;
    
    UInt_t tmpWord;
    
    TTree* unsortedTree = new TTree( "unsortedTree", baseName );
    unsortedTree->SetDirectory(0);
    
    unsortedTree->Branch( "channelID", &channelID, "channelID/s" );
    unsortedTree->Branch( "timestamp", &timestamp, "timestamp/l" );
    unsortedTree->Branch( "peakHighIndex", &peakHighIndex, "peakHighIndex/s" );
    unsortedTree->Branch( "peakHighValue", &peakHighValue, "peakHighValue/s" );
    unsortedTree->Branch( "formatBits", &formatBits, "formatBits/b" );
    unsortedTree->Branch( "accumulatorSum", accumulatorSum, "accumulatorSum[8]/i" );
    unsortedTree->Branch( "informationBits", &informationBits, "informationBits/b" );
    unsortedTree->Branch( "mawMaximumValue", &mawMaximumValue, "mawMaximumValue/i" );
    unsortedTree->Branch( "mawValueAfterTrigger", &mawValueAfterTrigger, "mawValueAfterTrigger/i" );
    unsortedTree->Branch( "mawValueBeforeTrigger", &mawValueBeforeTrigger, "mawValueBeforeTrigger/i" );
    unsortedTree->Branch( "mawTestData", &mawTestData, "mawTestData/i" );
    unsortedTree->Branch( "startEnergyValue", &startEnergyValue, "startEnergyValue/i" );
    unsortedTree->Branch( "maxEnergyValue", &maxEnergyValue, "maxEnergyValue/i" );
    unsortedTree->Branch( "mawTestFlag", &mawTestFlag, "mawTestFlag/O" );
    unsortedTree->Branch( "pileupFlag", &pileupFlag, "pileupFlag/O" );
    unsortedTree->Branch( "nSamples", &nSamples, "nSamples/i" );
    unsortedTree->Branch( "waveform", &sortingWave );
    
    
//    we have this one around just to set the branch up - delete it
//    wave->Delete();

    

    
	TFile *outFile = new TFile( outfileName,
                               "RECREATE", baseName );
    

    TTree* sis3316tree = (TTree*)unsortedTree->CloneTree( 0 );
    sis3316tree->SetName( "sis3316tree" );
    sis3316tree->SetDirectory( outFile );
    

    
    
    
    
    //
	//	keep track of the time taken to process things
    // start a timer now
    //
	Long64_t index = 0;
	time_t startTime, endTime;
    time_t processStartingTime;
	time( &processStartingTime );
    
    UInt_t packetWords;
    UInt_t eventWordsFromFormatBits;
    

    Long64_t spillNumber = 0;
    
    
    inFile.open( inputFilename.Data(), ifstream::in );
    
    printf( "Processing %s...\n", inputFilename.Data() );
    
    // seek past the header at the start of the binary file
    // 100 words = 400 bytes
    inFile.seekg( 400 );
    
    printf( "Header seeked through..\n" );
    
    while( inFile.good() ) {
        // we're at the start of a spill
        // read 10 word spill header
        inFile.read( bufferPointer, 40 );
        
        if( DEBUG ) {
            // if we're debugging, print out the first 10 words of the first spill
            for( Int_t i = 0; i < 10; i++ ) {
                memcpy( &tmpWord, &readBuffer[i], 4 );
                printf( "Word %i of %llu spill: \t %x\n", i, spillNumber, tmpWord );
            }
        }
        
        // check for EOF
        memcpy( &tmpWord, &readBuffer[0], 4 );
        if( tmpWord == 0x0E0F0E0F ) {
            break;
        }
        
        // skip the packet header for the spill
        // this is two words
        inFile.read( bufferPointer, 8 );
        
        
        
        for (Int_t cardNumber = 0; cardNumber < numCards; cardNumber++) {
        
        	// now that we're inside of the spill, we have to parse data for each channel
        	for( Int_t channelNumber = 0; channelNumber < channelsPerCard; channelNumber++ ) {
            
				// read in the packet header for the channel
				inFile.read( bufferPointer, 32 );
			
				// from the channel info packet header, we can determine the size of the packet data
				memcpy( &tmpWord, &readBuffer[7], 4 );
				packetWords = tmpWord;
			
				if( DEBUG ) {
					printf( "Number of words in packet for channel %i in spill %llu: \t %u\n", channelNumber, spillNumber, packetWords );
				}
			
				while( packetWords > 0 ) {
				
				
					if( index % 100000 == 0 ) {
						time(&endTime);
						printf( "Processed %lli events in %i seconds\n", index, (Int_t)difftime(endTime, processStartingTime ));
					}
				
					// first two words of an event are there no matter what the format bits are set to
					inFile.read( bufferPointer, 8 );
					packetWords -= 2;
					memcpy( &tmpWord, &readBuffer[0], 4 );
				
					formatBits = (UChar_t)(tmpWord & 0xf);
				
					channelID = (UShort_t)((tmpWord & 0xfff0) >> 4);
				
					timestamp = (ULong64_t)(tmpWord & 0xffff0000) << 16;
				
				
					memcpy( &tmpWord, &readBuffer[1], 4 );
				
					timestamp = timestamp | tmpWord;
				
				
					if( DEBUG ) {
						// print out the first two words of the event
						memcpy( &tmpWord, &readBuffer[0], 4 );
						printf("First two words of event:\t %x\t", tmpWord );
						memcpy( &tmpWord, &readBuffer[1], 4 );
						printf( "%x\n", tmpWord );
					}
				
					if( DEBUG ) {
						// print out the determined format bits
						printf( "Format bits: %i\n", formatBits );
					}
				
				
					// now based on format bits we can determine the number of words to read per event
					eventWordsFromFormatBits = 0;
					if( (formatBits & 0x1) != 0 ) {
					
						if( DEBUG ) {
							printf( "Reading words for format bit 0\n");
						}
						eventWordsFromFormatBits += 7;
						inFile.read( bufferPointer, 7 * 4 );
						packetWords -= 7;
						memcpy( &tmpWord, &readBuffer[0], 4);
						peakHighValue = tmpWord & 0xffff;
						peakHighIndex = (tmpWord & 0xffff0000) >> 16;
						if( DEBUG ) {
							printf("Peak index: %i peak value: %i\n", peakHighIndex, peakHighValue );
						}
						memcpy( &tmpWord, &readBuffer[1], 4 );
						informationBits = ( tmpWord & 0xff000000 ) >> 24;
						accumulatorSum[0] = ( tmpWord & 0xffffff );
						memcpy( &accumulatorSum[1], &readBuffer[2], 4 );
						memcpy( &accumulatorSum[2], &readBuffer[3], 4 );
						memcpy( &accumulatorSum[3], &readBuffer[4], 4 );
						memcpy( &accumulatorSum[4], &readBuffer[5], 4 );
						memcpy( &accumulatorSum[5], &readBuffer[6], 4 );
					}
					else {
						peakHighIndex = 0;
						peakHighValue = 0;
						informationBits = 0;
						accumulatorSum[0] = 0;
						accumulatorSum[1] = 0;
						accumulatorSum[2] = 0;
						accumulatorSum[3] = 0;
						accumulatorSum[4] = 0;
						accumulatorSum[5] = 0;
					}
					if( (formatBits & 0x2) != 0 ) {
						if( DEBUG ) {
							printf( "Reading words for format bit 1\n");
						}
						eventWordsFromFormatBits += 2;
						inFile.read( bufferPointer, 2 * 4 );
						packetWords -= 2;
						memcpy( &accumulatorSum[6], &readBuffer[0], 4 );
						memcpy( &accumulatorSum[7], &readBuffer[1], 4 );
					}
					else {
						// populate accumulators with 0 if they're not defined
						accumulatorSum[6] = 0;
						accumulatorSum[7] = 0;
					}
					if( (formatBits & 0x4) != 0 ) {
						if( DEBUG ) {
							printf( "Reading words for format bit 2\n");
						}
						eventWordsFromFormatBits += 3;
						inFile.read( bufferPointer, 3 * 4 );
						packetWords -= 3;
						memcpy( &mawMaximumValue, &readBuffer[0], 4 );
						memcpy( &mawValueAfterTrigger, &readBuffer[1], 4 );
						memcpy( &mawValueBeforeTrigger, &readBuffer[2], 4 );
					}
					else {
						mawMaximumValue = 0;
						mawValueAfterTrigger = 0;
						mawValueBeforeTrigger = 0;
					}
					if( (formatBits & 0x8) != 0 ) {
						if( DEBUG ) {
							printf( "Reading words for format bit 3\n");
						}
						eventWordsFromFormatBits += 2;
						inFile.read( bufferPointer, 2 * 4 );
						packetWords -= 2;
						memcpy( &startEnergyValue, &readBuffer[0], 4 );
						memcpy( &maxEnergyValue, &readBuffer[1], 4 );
					}
					else {
						startEnergyValue = 0;
						maxEnergyValue = 0;
					}
				
					// the next word will determine the number of sample words we read
					inFile.read( (char*)&tmpWord, 4 );
					packetWords -= 1;
				
				
				
					nSamples = 2 * (tmpWord & 0x3ffffff);
				
					if( DEBUG ) {
						printf( "Determined there are %i sample words\n", nSamples );
					}
				
				
					pileupFlag = (tmpWord & 0x4000000 ) >> 26;
					mawTestFlag = ( tmpWord & 0x8000000 ) >> 27;
				
	//                for( Int_t i = 0; i < (nSamples / 2 ); i++ ) {
	//                    inFile.read( (char*)&tmpWord, 4 );
	//                    packetWords -= 1;
	//                    waveform[i*2] = tmpWord & 0xffff;
	//                    waveform[i*2 + 1] = (tmpWord & 0xffff0000) >> 16;
	//                }
				
					wave = getWaveformForChannel( &inFile, &packetWords, nSamples );
					unsortedTree->SetBranchAddress("waveform", &wave);
				
					//                    inFile.read( (char*)&mawTestData, 4 );
					//                    packetWords -= 1;
					mawTestData = 0;
				
				
					//If run stopped during spill writing, the program can crash
					//This isn't a great fix, since we will have partial data from the 
					//channels before this and nothing from this or channels after, but
					//it will allow it to run. A better solution is to only add files to
					//the TTree once we're sure we've reached the end of the last channel 
					//in the spill
					if (inFile.fail()) {
						break;
					}

				
					unsortedTree->Fill();
					index++;
				
					wave->Delete();
				}
			}
        }
        
        
        
        
        // now we do the time ordering of the spill
        if( DEBUG ) {
            printf( "Creating TTree index for spill %lli\n", spillNumber );
            time(&startTime);
        }
        unsortedTree->BuildIndex( "timestamp" );
        TTreeIndex* treeIndex = (TTreeIndex*)unsortedTree->GetTreeIndex();
        if( DEBUG ) {
            time(&endTime);
            printf( "Index created for spill %lli in %i seconds\n", spillNumber, (Int_t)difftime(endTime, startTime) );
        }
        
        if( DEBUG ) {
            printf( "Tree index contains %lli entries\n", treeIndex->GetN() );
        }
        
        unsortedTree->SetBranchAddress( "waveform", &sortingWave );
        sis3316tree->SetBranchAddress("waveform", &sortingWave );
        
        for( Int_t i = 0; i < treeIndex->GetN(); i++ ) {
            
            unsortedTree->GetEntry( treeIndex->GetIndex()[i] );
            
            if( DEBUG ) {
                printf( "%i-th event recalled is index %lli\n", i, treeIndex->GetIndex()[i] );
            }
            
            sis3316tree->Fill();
            
        }
        
        
        
        
        spillNumber++;
        
        treeIndex->Delete();
        unsortedTree->Reset();
        
    }
    
    inFile.close();

    
    time( &endTime );
	cout << "Recorded " << index << " events in "
    << difftime( endTime, processStartingTime ) << " seconds" << endl << endl;
    
    
    

    
    unsortedTree->Delete();
    sis3316tree->Write("sis3316tree", TObject::kOverwrite);
    outFile->Close();
    
    
    delete [] waveformBuffer;
}