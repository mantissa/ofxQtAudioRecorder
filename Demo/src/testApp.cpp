
#include "testApp.h"

//--------------------------------------------------------------

testApp::~testApp(){
	
	// clear audio buffers
	
	delete [] left;
	delete [] right;
	delete [] audioBufferFloat;
}

//--------------------------------------------------------------

void testApp::setup(){

	ofSetWindowShape(1080, 510);
	
	// you may need to configure your system audio input
	// get a list of devices and pick the one you want to use
	// soundStream.listDevices();
	// soundStream.setDeviceID(6);
	
	// create audio buffers ...
	// 0 output channels, 
	// 2 input channels
	// 44100 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	ofSoundStreamSetup(0, NUM_CHANNELS, this, SAMPLE_RATE, BUFFER_SIZE, 4);
	
	// set flag and counter
	bIsRecording = false;
	bufferCounter = 0;
	
	// create L/R audio channels
	left = new float[BUFFER_SIZE];
	right = new float[BUFFER_SIZE];
	
	// set values to zero
	memset(left, 0, BUFFER_SIZE);
	if( NUM_CHANNELS == 2 ) memset(right, 0, BUFFER_SIZE);
	
	// create an empty buffer (stereo channels are interlaced)
	audioBufferFloat = new float[SAMPLE_RATE * RECORDING_DURATION * NUM_CHANNELS];
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){
	
	// draw the most recent audio samples
	
	ofSetHexColor(0x333333);
	ofRect(20,20,256,200);
	ofSetHexColor(0xFFFFFF);
	
	// a) draw the left
	for (int i = 0; i < BUFFER_SIZE; i++){
		ofLine(20+i,120,20+i,120+left[i]*100.0f);
	}
	
	if(NUM_CHANNELS==2){
	
		// b) draw the right
		ofSetHexColor(0x333333);
		ofRect(300,20,256,200);
		ofSetHexColor(0xFFFFFF);
		for (int i = 0; i < BUFFER_SIZE; i++){
			
			ofLine(300+i,120,300+i,120+right[i]*100.0f);
		}
	}
	
	// draw the recording waveform (what we've saved so far ...)
	
	int waveformWidth = ofGetWidth()-40;
	
	ofSetHexColor(0x333333);
	ofRect(20,240,waveformWidth,200);
	ofSetHexColor(0xFFFFFF);
	
	if(NUM_CHANNELS==1){
	
		for (int i = 0; i < bufferCounter * BUFFER_SIZE; i++){
			
			float xPos = 20+ofMap(i, 0, RECORDING_DURATION * SAMPLE_RATE, 0, waveformWidth);
			float offset = audioBufferFloat[i]*100;
			
			ofLine(xPos,340,xPos,340+offset);
		}
		
	} else {
	
		for (int i = 0; i < bufferCounter * BUFFER_SIZE; i++){
			
			float xPos = 20+ofMap(i, 0, RECORDING_DURATION * SAMPLE_RATE, 0, waveformWidth);
			float offset = audioBufferFloat[i*2]*100;
			
			ofLine(xPos,340,xPos,340+offset);
		}
		
	}

	// feedback on the recording
	ofSetHexColor(0x333333);
	char reportString[255];
	sprintf(reportString, "Samples recorded: %i\n\n", bufferCounter * BUFFER_SIZE);
	ofDrawBitmapString(reportString, 20, 490);
	ofDrawBitmapString("Press SPACEBAR to record", 20, 470);
}

//--------------------------------------------------------------

void testApp::startMovie(){
	
	bufferCounter = 0;
	audioRecorder.createMovie("ofAudioRecording.mov", SAMPLE_RATE, NUM_CHANNELS);	
	bIsRecording = true;
}

//--------------------------------------------------------------

void testApp::finishMovie(){
	
	bIsRecording = false;
	audioRecorder.addAudioSamples(audioBufferFloat, SAMPLE_RATE * RECORDING_DURATION * NUM_CHANNELS);
	audioRecorder.closeMovie();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	if(key == ' ' && !bIsRecording){
		
		startMovie();
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::audioReceived(float * input, int bufferSize, int nChannels){	
	
	// All the work happens here
	// This function is automatically called when ofSoundStreamSetup() is called
	// and new audio samples arrive (depends on the sample rate)
	
	if(bIsRecording){
		
		if(NUM_CHANNELS == 1){
			
			// single channel recording
		
			for (int i = 0; i < bufferSize; i++){
				
				// copy the samples to the left channel
				left[i] = input[i];
				
				// append samples to saved buffer
				audioBufferFloat[bufferCounter * bufferSize * nChannels+i*nChannels] =  input[i*nChannels];
			}
		
			
		} else if( NUM_CHANNELS == 2 ){
			
			// assuming 2 channels
			
			for (int i = 0; i < bufferSize; i++){
				
				// un-weave channels
				left[i] = input[i*nChannels];
				right[i] = input[i*nChannels+1];
				
				// append samples to saved buffer ( interleaved)
				audioBufferFloat[bufferCounter * bufferSize * nChannels+i*nChannels] =  input[i*nChannels];
				audioBufferFloat[bufferCounter * bufferSize * nChannels+i*nChannels+1] =  input[i*nChannels+1];
			}
		
		} else {
		
			// no logic for more than stereo
		}

		// how many samples have to saved
		bufferCounter++;
		
		int maxBuffers = SAMPLE_RATE * RECORDING_DURATION / bufferSize;
		
		if(bufferCounter >= maxBuffers){
			
			//printf("recorded %i/%i buffers\n", bufferCounter, maxBuffers);
		
			finishMovie();
		}
		
	} else {
		
		// display the previously recorded buffer when not recording
		
		if(NUM_CHANNELS == 1){
			
			// copy the samples to the left channel
			for (int i = 0; i < bufferSize; i++){
				
				left[i] = input[i];
			}
			
		} else {
		
			// samples are "interleaved"
			for (int i = 0; i < bufferSize; i++){
				
				// un-weave channels
				left[i] = input[i*nChannels];
				right[i] = input[i*nChannels+1];
			}
		}
	}
}