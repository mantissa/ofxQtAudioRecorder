

#ifndef OFX_QUICKTIME_AUDIO_RECORDER_H
#define OFX_QUICKTIME_AUDIO_RECORDER_H

#include "ofMain.h"
#include "ofQtUtils.h"
#import <CoreAudio/CoreAudio.h>

// Quicktime Audio Recording in openFrameworks
// from example here: http://developer.apple.com/mac/library/qa/qa2007/qa1539.html
// this was helpful too: http://archive.mildmanneredindustries.com/archive/Xmeeting/20070721/expanded/XMeeting/Source/Source/Framework/XMCallRecorder.m

class ofxQtAudioRecorder {
	
	public:
	
		ofxQtAudioRecorder();
	
		void createMovie(string url, int sampleRate, int numChannels);
		void addAudioSamples(float * audioBuffer, int sampleCount);
		void closeMovie();
	
	protected:
	
		void prepareAudioRecording(int sampleRate, int numChannels);
	
		// quicktime movie file guts
		Movie movie;
		short movieResRefNum;
		Track videoTrack;
		Media videoMedia;
		Track audioTrack;
		Media audioMedia;
	
		int numChannels;
	
		// data references
		Handle dataRef;
		DataHandler dataHandler;
	
		// file stuff
		FSSpec myFileSpec;
	
		// audio files
		SoundDescriptionHandle soundDesc;
};

#undef checkErr
#define checkErr(theCode) \
{ \
if(err != noErr) { \
printf("error: %p\n", theCode); \
} \
}
#endif