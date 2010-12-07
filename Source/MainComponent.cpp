/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  1 Dec 2010 11:30:39pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "MainComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MainComponent::MainComponent ()
    : mOpenFileButton (0),
      mSaveFileButton (0),
	  mCurrentAudioFileSource(0),
	  mInterleavedBuffer(0),
	  mLeftBuffer(0),
	  mRightBuffer(0)
{
    addAndMakeVisible (mOpenFileButton = new TextButton (T("OpenFileButton")));
    mOpenFileButton->setButtonText (T("Open File"));
    mOpenFileButton->addButtonListener (this);

    addAndMakeVisible (mSaveFileButton = new TextButton (T("SaveFileButton")));
    mSaveFileButton->setButtonText (T("Save File"));
    mSaveFileButton->addButtonListener (this);


    //[UserPreSize]
	mDeviceManager.initialise (2, 2, 0, true, String::empty, 0);
	mDeviceManager.addAudioCallback (&mAudioSourcePlayer);
    mAudioSourcePlayer.setSource (&mTransportSource);
    mCurrentAudioFileSource = 0;
    //[/UserPreSize]


    setSize (1024, 768);

    //[Constructor] You can add your own custom stuff here..
	//setup Grain variables for testing
	mGrainLength = 44100/8;	//smaller value is smaller grain lengths and less recognizable sounds
	mGrainCurrentPositionRelativeLeft = 0;
	mGrainCurrentPositionRelativeRight = 0;
	mGrainStartPositionAbsolute = 0;
	mSampleCounter = 0;
	mVelocityFactor = 2.0f;	//higher is slower moving through audio file
	mGrainAdvanceAmount = mGrainLength/2;//how many samples the window will advance through the audio file
    //[/Constructor]
}

MainComponent::~MainComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (mOpenFileButton);
    deleteAndZero (mSaveFileButton);

    //[/Destructor_pre]

    //[Destructor]. You can add your own custom destruction code here..
	mTransportSource.setSource (0);
    mAudioSourcePlayer.setSource (0);

    mDeviceManager.removeAudioCallback (&mAudioSourcePlayer);

    deleteAndZero (mCurrentAudioFileSource);
	if (mLeftBuffer != 0)
	{
		free(mLeftBuffer);
		mLeftBuffer = 0;
	}
	if (mRightBuffer != 0)
	{
		free(mRightBuffer);
		mRightBuffer = 0;
	}
    //[/Destructor]
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainComponent::resized()
{
    mOpenFileButton->setBounds (24, 272, 150, 24);
    mSaveFileButton->setBounds (216, 272, 150, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MainComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == mOpenFileButton)
    {
		 //[UserButtonCode_mOpenFileButton] -- add your button handler code here..
		WildcardFileFilter wildcardFilter ("*.wav","", "Wave files");

        FileBrowserComponent browser (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                                      File::nonexistent,
                                      &wildcardFilter,
                                      0);

        FileChooserDialogBox dialogBox ("Open a Wave File",
                                        "Please choose a Wave file to open...",
                                        browser, true,
                                        Colours::lightblue);

        if (dialogBox.show())
        {
            File mCurrentFile = browser.getSelectedFile(0);
			//playAudioFile(mCurrentFile);
			memStoreAudioFile(mCurrentFile);
			mDeviceManager.addAudioCallback(this);
        }
       
        //[/UserButtonCode_mOpenFileButton]
    }
    else if (buttonThatWasClicked == mSaveFileButton)
    {
        //[UserButtonCode_mSaveFileButton] -- add your button handler code here..
        //[/UserButtonCode_mSaveFileButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void MainComponent::memStoreAudioFile(File &audioFile)
{
	//we only want raw samples, no header
	AudioFormatManager formatManager;
	formatManager.registerBasicFormats();
	AudioFormatReader *reader = formatManager.createReaderFor(audioFile);
	if (reader != 0)
	{
		if (mLeftBuffer != 0)
		{
			free(mLeftBuffer);
			mLeftBuffer = 0;
		}
		if (mRightBuffer != 0)
		{
			free(mRightBuffer);
			mRightBuffer = 0;
		}
		int num = reader->numChannels;
		int* destBufs[num];
		mLeftBuffer = (float*)malloc(reader->lengthInSamples/reader->numChannels*sizeof(float));
		destBufs[0] = (int*)mLeftBuffer;
		if (reader->numChannels > 1)
		{
			mRightBuffer = (float*)malloc(reader->lengthInSamples/reader->numChannels*sizeof(float));
			destBufs[1] = (int*)mRightBuffer;
		}
		reader->read(destBufs, reader->numChannels, 0, reader->lengthInSamples/reader->numChannels, false);
		mBufferLength = reader->lengthInSamples;
		mNumChannels = reader->numChannels;
		
		for (int i=0; i<(mBufferLength/mNumChannels); i++)
		{
			//scale to -1.0 to 1.0 floats
			mLeftBuffer[i] = ((float)destBufs[0][i])/(1.0f*INT_MAX);
			if (mRightBuffer != 0)
				mRightBuffer[i] = ((float)destBufs[1][i])/(1.0f*INT_MAX);
		}
		delete reader;
	}
}

void MainComponent::saveAudioFile(File &saveFile)
{
	int num = 1;
	if (mRightBuffer != 0)
		num++;
	int* destBufs[num];
	destBufs[0] = (int*)mLeftBuffer;
	if (mRightBuffer)
		destBufs[1] = (int*)mRightBuffer;
	
	saveFile.deleteFile();
	OutputStream *output = saveFile.createOutputStream();
	WavAudioFormat wavformat;
	AudioFormatWriter *writer = wavformat.createWriterFor(output, 44100.0, num, 16, StringPairArray(), 0);
	writer->write((const int**)destBufs, mBufferLength);
	delete writer;	
}

void MainComponent::playAudioFile(File &audioFile)
{
 // unload the previous file source and delete it..
    mTransportSource.stop();
    mTransportSource.setSource (0);
    deleteAndZero (mCurrentAudioFileSource);

    // get a format manager and set it up with the basic types (wav and aiff).
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    AudioFormatReader* reader = formatManager.createReaderFor (audioFile);

    if (reader != 0)
    {
        mCurrentAudioFileSource = new AudioFormatReaderSource (reader, true);

        // ..and plug it into our transport source
        mTransportSource.setSource (mCurrentAudioFileSource,
                                   32768, // tells it to buffer this many samples ahead
                                   reader->sampleRate);
		mTransportSource.setPosition (0);
		mTransportSource.start();
    }
}

void MainComponent::audioDeviceAboutToStart (AudioIODevice* device)
{
    zeromem (samples, sizeof (samples));
}

void MainComponent::audioDeviceStopped()
{
    zeromem (samples, sizeof (samples));
}

void MainComponent::audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
												 float** outputChannelData, int numOutputChannels, int numSamples)
{
	// We need to clear the output buffers, in case they're full of junk..
    for (int i = 0; i < numOutputChannels; ++i)
	{
        if (outputChannelData[i] != 0)
            zeromem (outputChannelData[i], sizeof (float) * numSamples);
	}	 
	if (renderAudioToBuffer(outputChannelData, numOutputChannels, numSamples))
	{
		mDeviceManager.removeAudioCallback(this);
	}
}

bool MainComponent::renderAudioToBuffer(float** outputChannelData, int numOutputChannels, int numSamples)
{
	// This renders the granular audio into the output buffer for playback or file writing
	for (int i = 0; i < numOutputChannels; ++i)
	{
		float *output = outputChannelData[i];
		int samplesToRead = numSamples;
		if ((mGrainStartPositionAbsolute + mGrainCurrentPositionRelativeLeft + numSamples) > mBufferLength/mNumChannels)
		{
			//do something here, we're about to go out of bounds
			 zeromem (outputChannelData[0], sizeof (float) * numSamples);
			 zeromem (outputChannelData[1], sizeof (float) * numSamples);
			 mGrainStartPositionAbsolute = 0;
			 mGrainCurrentPositionRelativeLeft = 0;
			 mGrainCurrentPositionRelativeRight = 0;
			 //returns true if we've hit the end of our data.  The caller should stop playing or writing audio here
			 // This will just kill the last audio buffer instead of silencing the remaining space with zeros.  
			 // Better to silence with zeros, but that's more work!
			 return true;
		}
		
		for (int j=0; j<samplesToRead; j++)
		{
			if (i==0)//left channel
			{
				int samppos = mGrainStartPositionAbsolute + mGrainCurrentPositionRelativeLeft;
				output[j] = mLeftBuffer[samppos];
				mGrainCurrentPositionRelativeLeft++;
				if (mGrainCurrentPositionRelativeLeft >= mGrainLength)
					mGrainCurrentPositionRelativeLeft = 0;
			
			}
			else	//right channel
			{
				int samppos = mGrainStartPositionAbsolute + mGrainCurrentPositionRelativeRight;
				output[j] = mRightBuffer[samppos];
				mGrainCurrentPositionRelativeRight++;
				if (mGrainCurrentPositionRelativeRight >= mGrainLength)
					mGrainCurrentPositionRelativeRight = 0;
			}
	
		}
	}
	
	mSampleCounter += numSamples;
	if (mSampleCounter >= (int64)(mGrainLength * mVelocityFactor))
	{
		mGrainStartPositionAbsolute += mGrainAdvanceAmount;
		mSampleCounter = 0;
	}
	
	return false;
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TEXTBUTTON name="OpenFileButton" id="e02073e0540adda" memberName="mOpenFileButton"
              virtualName="" explicitFocusOrder="0" pos="24 272 150 24" buttonText="Open File"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="SaveFileButton" id="3f9e2515deae5ca9" memberName="mSaveFileButton"
              virtualName="" explicitFocusOrder="0" pos="216 272 150 24" buttonText="Save File"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif