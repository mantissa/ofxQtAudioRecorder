#ifndef _TEST_APP
#define _TEST_APP

/* --------------------------------------------------------------
 
Recording system audio in Apple QuickTime movies
Jeremy Rotsztain / www.mantissa.ca / jeremy@mantissa.ca
August 30, 2010
 
NOTE: Verify you system audio settings. You may get unusual results if your sample rates differ
 
-------------------------------------------------------------- */ 

#include "ofMain.h"
#include "ofxQtAudioRecorder.h"

#define SAMPLE_RATE				44100
#define BUFFER_SIZE				256
#define RECORDING_DURATION		3
#define NUM_CHANNELS			1

class testApp : public ofBaseApp{

	public:
	
		~testApp();
	
		void setup();
		void update();
		void draw();
	
		void startMovie();
		void finishMovie();
	
		void keyPressed  (int key);
		void keyReleased(int key);
	
		void audioReceived 	(float * input, int bufferSize, int nChannels); 
	
		// audio buffer
		float * audioBufferFloat;
	
		// stereo buffer
		float * left;
		float * right;
		int 	bufferCounter;
	
		bool bIsRecording;

		ofxQtAudioRecorder audioRecorder;
		ofSoundStream soundStream;
};




#endif
