
#include "ofxQtAudioRecorder.h"

// Constants for use when creating our movie track and media

static const TimeValue  kSoundSampleDuration    = 1;
static const TimeValue  kTrackStart             = 0;
static const TimeValue  kMediaStart             = 0;

//--------------------------------------------------------------
// ofxQtAudioRecorder()
// make sure quicktime is initialized
//--------------------------------------------------------------

ofxQtAudioRecorder::ofxQtAudioRecorder(){
	
	initializeQuicktime();
}

//--------------------------------------------------------------
// createMovie()
// create a quicktime movie file
//--------------------------------------------------------------

void ofxQtAudioRecorder::createMovie(string url, int sampleRate, int numChannels){
	
	OSErr					err = noErr;
	FSRef					fsref;
	Boolean					isdir;
	
	// full data path
	url = ofToDataPath(url, true);
	
	// kill a file and make a new one if needed:	
	FILE * pFile;
	pFile = fopen (url.c_str(),"w");
	fclose (pFile);
	
	// create path reference
	err = FSPathMakeRef((const UInt8*)url.c_str(), &fsref, &isdir);
	if (err) { printf("FSPathMakeRef failed %d\n", err); printf("ERROR creating movie \n"); return; }
	
	// create file spec
	err = FSGetCatalogInfo(&fsref, kFSCatInfoNone, NULL, NULL, &myFileSpec, NULL);
	if (err) { printf("FSGetCatalogInfo failed %d\n", err); printf("ERROR creating movie \n"); return; }
	
	// create that movie file
	err = CreateMovieFile(&myFileSpec, 'TVOD', smCurrentScript, createMovieFileDeleteCurFile | createMovieFileDontCreateResFile, &movieResRefNum, &movie);
	if(err) {
		printf("error: failed to create movie file\n");
		return;
	}
	
	prepareAudioRecording(sampleRate, numChannels);
}

//--------------------------------------------------------------
// prepareAudioRecording()
// setup audio file for recording
// note: assumes audio codec is Linear PCM
//--------------------------------------------------------------

void ofxQtAudioRecorder::prepareAudioRecording(int sampleRate, int numChannels_){
	
	OSErr err = noErr;
	
	// create audio description and set format ID along with format flags, mBitsPerChannel and mBytesPerPacket
	AudioStreamBasicDescription asbd;
	
	numChannels = numChannels_;
	
	asbd.mSampleRate           = sampleRate;
    asbd.mFormatID             = kAudioFormatLinearPCM;
    asbd.mFormatFlags          = kAudioFormatFlagsNativeFloatPacked;
    // if multi-channel, the data format must be interleaved (non-interleaved is not allowed), 
    // and you should set up the asbd accordingly
    asbd.mChannelsPerFrame     = numChannels; // 2 (Stereo)
    // mBitsPerChannel = number of bits of sample data for each channel in a frame of data
    asbd.mBitsPerChannel       = sizeof (Float32) * 8; // was 8 // 32-bit floating point PCM
    // mBytesPerFrame = number of bytes in a single sample frame of data
    // (bytes per channel) * (channels per frame) = 4 * 2 = 8
    asbd.mBytesPerFrame        = (asbd.mBitsPerChannel>>3) * asbd.mChannelsPerFrame; // // number of *bytes* per channel * channels per frame
    asbd.mFramesPerPacket      = 1; // For PCM, frames per packet is always 1
    // mBytesPerPacket = (bytes per frame) * (frames per packet) = 8 * 1 = 8
    asbd.mBytesPerPacket       = asbd.mBytesPerFrame * asbd.mFramesPerPacket;
	
	//printf("bits per channel %i\n", asbd.mBitsPerChannel);
	//printf("bytes per frame %i - %i %i\n", asbd.mBytesPerFrame, (asbd.mBitsPerChannel>>3), asbd.mChannelsPerFrame);
	//printf("bytes per packet %i\n", asbd.mBytesPerPacket);
	
	err = QTSoundDescriptionCreate(&asbd,
								   NULL, 0,
								   NULL, 0,
								   kQTSoundDescriptionKind_Movie_Version2,
								   &soundDesc);
	checkErr(0x0200);
	
	audioTrack = NewMovieTrack(movie, Long2Fix(0), Long2Fix(0), kFullVolume);
	err = GetMoviesError();
	checkErr(0x0201);
	
	audioMedia = NewTrackMedia(audioTrack, SoundMediaType, sampleRate, NULL, 0);
	err = GetMoviesError();
	checkErr(0x0202);
	
	return;	
}

//--------------------------------------------------------------
// addAudioSamples()
// add all audio data for recording
// note: do this once -- at the end of the recording
//--------------------------------------------------------------

void ofxQtAudioRecorder::addAudioSamples( float * audioBuffer, int sampleCount){
	
	if(audioMedia != NULL)
	{
		
		printf("adding %i samples\n", sampleCount);
				
		OSErr err = BeginMediaEdits(audioMedia);
		checkErr(0x0201);
		
		int bytesPerSample = sizeof(Float32);
		
		//printf("bytes %i\n", bytesPerSample);
		
		AddMediaSample2(audioMedia,   // insert into audio media
						(UInt8*)audioBuffer, // size
						sampleCount * bytesPerSample, // number of bytes of audio data
						kSoundSampleDuration,  // normal decode duration
						0,            // no display offset (won't work anyway)
						(SampleDescriptionHandle) soundDesc,
						sampleCount / numChannels, // number of samples 
						0,            // no flags
						NULL);        // not interested in decode time
		
		// end media editing and add media to the track
		EndMediaEdits(audioMedia);
		checkErr(0x0202);
		
		TimeValue trackStart = (TimeValue)(0); 
		InsertMediaIntoTrack(audioTrack, trackStart, 0, GetMediaDuration(audioMedia), fixed1);
		checkErr(0x0203);
		
		//printf("added audio media\n");
	}
}

//--------------------------------------------------------------
// closeMovie()
// Closes QuickTime movie and structures for recording
//--------------------------------------------------------------

void ofxQtAudioRecorder::closeMovie(){
	
	short movieResId = movieInDataForkResID;
	AddMovieResource(movie, movieResRefNum, &movieResId, NULL);
	OSErr err = GetMoviesError();
	
	if(err != noErr){
		
		printf("error adding movie resource\n");
		return;
	}
	
	if(movieResRefNum != 0)
	{
		CloseMovieFile(movieResRefNum);
	}
	
	if(movie != NULL)
	{
		DisposeMovie(movie);
	}
	
	if(soundDesc != NULL)
	{
		DisposeHandle((Handle)soundDesc);
		soundDesc = NULL;
	}
	
	movie = NULL;
	movieResRefNum = 0;
	audioMedia = NULL;
	audioTrack = NULL;
	
	printf("closed movie file\n");
}