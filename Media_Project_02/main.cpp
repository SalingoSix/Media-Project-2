/*
==================================================================================================================================================================
|Copyright © 2017 Oscar Lara	- scarlara@hotmail.com																											   |
|																																								   |
|See individual libraries separate legal notices																												   |
|																																								   |
|Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"),				   |
|to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,			       |
|and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :				       |
|																																								   |
|The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.								       |
|																																								   |
|THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,			   |
|FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,	   |
|WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.	       |
|Based on FMOD example Effects																																		   |
==================================================================================================================================================================
*/


#include "src\utils\utils.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <string>
#include <fstream>
#include <sstream>

#define NUMBER_OF_SOUNDS 12
const float DISTANCEFACTOR = 1.0f;          // Units per meter.  I.e feet would = 3.28.  centimeters would = 100.
const int INTERFACE_UPDATE_TIME = 50;

//Globals

FMOD_RESULT mresult;
FMOD::System *msystem = NULL;
FMOD::Channel *pChannel[12];
FMOD::ChannelGroup *pChannelGroup[3];
FMOD::Sound *pSound[12];
std::string songNames[12];

bool             listenerflag = true;
FMOD_VECTOR      listenerpos = { 0.0f, 0.0f, -1.0f * DISTANCEFACTOR };

float songPan[12] = { 0 };
float songVolume[12] = { 1,1,1,1,1,1,1,1,1,1,1,1 };
float songPitch[12] = { 1,1,1,1,1,1,1,1,1,1,1,1 };
float songSpeed[12] = { 1,1,1,1,1,1,1,1,1,1,1,1 }; //Not useful unless sound file type supports speed changing

float mfrequency = 0.0f;
float mvolume = 0.0f;
float mpan = 0.0f;

bool mis_esc = false;
bool mkeydown = false;
bool playingMusic = 0;
unsigned int posInSong = 0;
unsigned int songLength = 0;

int currentChannel = 0;
int currentGroup = 0;
bool groupAdjust = true;

int refreshInfo = 50;

FMOD::Sound *msounds[NUMBER_OF_SOUNDS];
FMOD::Channel *mchannels[NUMBER_OF_SOUNDS];

//Master channel group
FMOD::ChannelGroup *mastergroup = 0;

//DSP variables
FMOD::DSP* dspLowPass1 = 0;		FMOD::DSP* dspLowPass2 = 0;		FMOD::DSP* dspLowPass3 = 0;
FMOD::DSP* dspHighPass1 = 0;	FMOD::DSP* dspHighPass2 = 0;	FMOD::DSP* dspHighPass3 = 0;
FMOD::DSP* dspFlange1 = 0;		FMOD::DSP* dspFlange2 = 0;		FMOD::DSP* dspFlange3 = 0;

void volumeUp(int chan);
void volumeDown(int chan);
void panLeft(int chan);
void panRight(int chan);
void speedUp(int chan);
void speedDown(int chan);
void pitchUp(int chan);
void pitchDown(int chan);

void errorcheck(FMOD_RESULT result) {
	if (result != FMOD_OK)
	{
		fprintf(stderr, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
}


void init_fmod() {
	
	// Create the main system object.
	mresult = FMOD::System_Create(&msystem);
	if (mresult != FMOD_OK)
	{
		fprintf(stderr, "FMOD error! (%d) %s\n", mresult, FMOD_ErrorString(mresult));
		exit(-1);
	}

	//Initializes the system object, and the msound device. This has to be called at the start of the user's program
	mresult = msystem->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
	if (mresult != FMOD_OK)
	{
		fprintf(stderr, "FMOD error! (%d) %s\n", mresult, FMOD_ErrorString(mresult));
		exit(-1);
	}

}


int sleep_ms = 100;
//You might need to update/optimize this code, 
//This implementation is good enough for demonstration purposes
//once we move to an opengl app, we will use a library to handle user input.
//Out main focus is FMOD low level api
void handle_keyboard() {

	//===============================================================================================
	//Esc key pressed
	if (GetAsyncKeyState(VK_ESCAPE)) {
		mis_esc = true;
	}

	//===============================================================================================
	// Space bar pressed		
	if ((GetKeyState(VK_SPACE) < 0) && !mkeydown) {		
		mkeydown = true;
		
		bool getPaused0, getPaused3, getPaused6;

		mresult = pChannel[0]->getPaused(&getPaused0);
		errorcheck(mresult);
		mresult = pChannel[3]->getPaused(&getPaused3);
		errorcheck(mresult);
		mresult = pChannel[6]->getPaused(&getPaused6);
		errorcheck(mresult);

		if (!getPaused0)
		{
			pChannel[0]->setPaused(true);
			pChannel[3]->setPaused(false);
			
		}
		else if (!getPaused3)
		{
			pChannel[3]->setPaused(true);
			pChannel[6]->setPaused(false);
		}

		else if (!getPaused6)
		{
			pChannel[6]->setPaused(true);
			pChannel[0]->setPaused(false);
		}

		Sleep(sleep_ms);
		mkeydown = false;
	}	


	//===============================================================================================
	// F1 Pressed		
	if ((GetKeyState(VK_F1) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 0;
		currentGroup = 0;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F2 Pressed		
	if ((GetKeyState(VK_F2) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 1;
		currentGroup = 0;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F3 Pressed		
	if ((GetKeyState(VK_F3) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 2;
		currentGroup = 0;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F4 Pressed		
	if ((GetKeyState(VK_F4) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 3;
		currentGroup = 1;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F5 Pressed		
	if ((GetKeyState(VK_F5) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 4;
		currentGroup = 1;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F6 Pressed		
	if ((GetKeyState(VK_F6) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 5;
		currentGroup = 1;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F7 Pressed		
	if ((GetKeyState(VK_F7) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 6;
		currentGroup = 2;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F8 Pressed		
	if ((GetKeyState(VK_F8) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 7;
		currentGroup = 2;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F9 Pressed		
	if ((GetKeyState(VK_F9) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 8;
		currentGroup = 2;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F10 Pressed		
	if ((GetKeyState(VK_F10) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 9;
		currentGroup = -1;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F11 Pressed		
	if ((GetKeyState(VK_F11) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 10;
		currentGroup = -1;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// F12 Pressed		
	if ((GetKeyState(VK_F12) < 0) && !mkeydown) {
		mkeydown = true;
		currentChannel = 11;
		currentGroup = -1;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	// SHIFT Pressed		
	if ((GetKeyState(VK_SHIFT) < 0) && !mkeydown) {
		mkeydown = true;
		groupAdjust = !groupAdjust;

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	//Arrow UP
	else if ((GetKeyState(VK_UP) < 0) && !mkeydown) {
		mkeydown = true;
		if (groupAdjust)
		{
			if (currentGroup == 0)
			{
				volumeUp(0);
				volumeUp(1);
				volumeUp(2);
			}
			else if (currentGroup == 1)
			{
				volumeUp(3);
				volumeUp(4);
				volumeUp(5);
			}
			else if (currentGroup == 2)
			{
				volumeUp(6);
				volumeUp(7);
				volumeUp(8);
			}
			else
				volumeUp(currentChannel);
		}
		else
			volumeUp(currentChannel);
		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Arrow Down
	else if ((GetKeyState(VK_DOWN) < 0) && !mkeydown) {
		mkeydown = true;
		if (groupAdjust)
		{
			if (currentGroup == 0)
			{
				volumeDown(0);
				volumeDown(1);
				volumeDown(2);
			}
			else if (currentGroup == 1)
			{
				volumeDown(3);
				volumeDown(4);
				volumeDown(5);
			}
			else if (currentGroup == 2)
			{
				volumeDown(6);
				volumeDown(7);
				volumeDown(8);
			}
			else
				volumeDown(currentChannel);
		}
		else
			volumeDown(currentChannel);
		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Arrow right
	else if ((GetKeyState(VK_RIGHT) < 0) && !mkeydown) {
		mkeydown = true;
		if (groupAdjust)
		{
			if (currentGroup == 0)
			{
				panRight(0);
				panRight(1);
				panRight(2);
			}
			else if (currentGroup == 1)
			{
				panRight(3);
				panRight(4);
				panRight(5);
			}
			else if (currentGroup == 2)
			{
				panRight(6);
				panRight(7);
				panRight(8);
			}
			else
				panRight(currentChannel);
		}
		else
			panRight(currentChannel);
		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Arrow left
	else if ((GetKeyState(VK_LEFT) < 0) && !mkeydown) {
		mkeydown = true;
		if (groupAdjust)
		{
			if (currentGroup == 0)
			{
				panLeft(0);
				panLeft(1);
				panLeft(2);
			}
			else if (currentGroup == 1)
			{
				panLeft(3);
				panLeft(4);
				panLeft(5);
			}
			else if (currentGroup == 2)
			{
				panLeft(6);
				panLeft(7);
				panLeft(8);
			}
			else
				panLeft(currentChannel);
		}
		else
			panLeft(currentChannel);
		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//O Key
	else if ((GetKeyState(0x4F) < 0) && !mkeydown) {
		mkeydown = true;
		if (groupAdjust)
		{
			if (currentGroup == 0)
			{
				speedDown(0);
				speedDown(1);
				speedDown(2);
			}
			else if (currentGroup == 1)
			{
				speedDown(3);
				speedDown(4);
				speedDown(5);
			}
			else if (currentGroup == 2)
			{
				speedDown(6);
				speedDown(7);
				speedDown(8);
			}
			else
				speedDown(currentChannel);
		}
		else
			speedDown(currentChannel);
		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//P Key
	else if ((GetKeyState(0x50) < 0) && !mkeydown) {
		mkeydown = true;
		if (groupAdjust)
		{
			if (currentGroup == 0)
			{
				speedUp(0);
				speedUp(1);
				speedUp(2);
			}
			else if (currentGroup == 1)
			{
				speedUp(3);
				speedUp(4);
				speedUp(5);
			}
			else if (currentGroup == 2)
			{
				speedUp(6);
				speedUp(7);
				speedUp(8);
			}
			else
				speedUp(currentChannel);
		}
		else
			speedUp(currentChannel);
		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//K Key
	else if ((GetKeyState(0x4B) < 0) && !mkeydown) {
		mkeydown = true;
		if (groupAdjust)
		{
			if (currentGroup == 0)
			{
				pitchDown(0);
				pitchDown(1);
				pitchDown(2);
			}
			else if (currentGroup == 1)
			{
				pitchDown(3);
				pitchDown(4);
				pitchDown(5);
			}
			else if (currentGroup == 2)
			{
				pitchDown(6);
				pitchDown(7);
				pitchDown(8);
			}
			else
				pitchDown(currentChannel);
		}
		else
			pitchDown(currentChannel);
		Sleep(sleep_ms);
		mkeydown = false;
	}
	
	//===============================================================================================
	//L Key
	else if ((GetKeyState(0x4C) < 0) && !mkeydown) {
		mkeydown = true;
		if (groupAdjust)
		{
			if (currentGroup == 0)
			{
				pitchUp(0);
				pitchUp(1);
				pitchUp(2);
			}
			else if (currentGroup == 1)
			{
				pitchUp(3);
				pitchUp(4);
				pitchUp(5);
			}
			else if (currentGroup == 2)
			{
				pitchUp(6);
				pitchUp(7);
				pitchUp(8);
			}
			else
				pitchUp(currentChannel);
		}
		else
			pitchUp(currentChannel);
		Sleep(sleep_ms);
		mkeydown = false;
	}
	
	//===============================================================================================
	//Q Key
	else if ((GetKeyState(0x51) < 0) && !mkeydown)
	{
		FMOD_VECTOR pos, vel;
		mresult = pChannel[9]->get3DAttributes(&pos, &vel);
		errorcheck(mresult);
		float soundPosX = pos.x;
		pos.x -= 1.0f * DISTANCEFACTOR;
		if (pos.x < -24 * DISTANCEFACTOR)
		{
			pos.x = -24 * DISTANCEFACTOR;
		}
		mresult = pChannel[9]->set3DAttributes(&pos, &vel);
		errorcheck(mresult);
	}

	//===============================================================================================
	//W Key
	else if ((GetKeyState(0x57) < 0) && !mkeydown)
	{
		FMOD_VECTOR pos, vel;
		mresult = pChannel[9]->get3DAttributes(&pos, &vel);
		errorcheck(mresult);
		float soundPosX = pos.x;
		pos.x += 1.0f * DISTANCEFACTOR;
		if (pos.x > 23 * DISTANCEFACTOR)
		{
			pos.x = 23 * DISTANCEFACTOR;
		}
		mresult = pChannel[9]->set3DAttributes(&pos, &vel);
		errorcheck(mresult);
	}
	
	//===============================================================================================
	//A Key
	else if ((GetKeyState(0x41) < 0) && !mkeydown)
	{
		FMOD_VECTOR pos, vel;
		mresult = pChannel[10]->get3DAttributes(&pos, &vel);
		errorcheck(mresult);
		float soundPosX = pos.x;
		pos.x -= 1.0f * DISTANCEFACTOR;
		if (pos.x < -24 * DISTANCEFACTOR)
		{
			pos.x = -24 * DISTANCEFACTOR;
		}
		mresult = pChannel[10]->set3DAttributes(&pos, &vel);
		errorcheck(mresult);
	}

	//===============================================================================================
	//S Key
	else if ((GetKeyState(0x53) < 0) && !mkeydown)
	{
		FMOD_VECTOR pos, vel;
		mresult = pChannel[10]->get3DAttributes(&pos, &vel);
		errorcheck(mresult);
		float soundPosX = pos.x;
		pos.x += 1.0f * DISTANCEFACTOR;
		if (pos.x > 23 * DISTANCEFACTOR)
		{
			pos.x = 23 * DISTANCEFACTOR;
		}
		mresult = pChannel[10]->set3DAttributes(&pos, &vel);
		errorcheck(mresult);
	}

	//===============================================================================================
	//Z Key
	else if ((GetKeyState(0x5A) < 0) && !mkeydown)
	{
		FMOD_VECTOR pos, vel;
		mresult = pChannel[11]->get3DAttributes(&pos, &vel);
		errorcheck(mresult);
		float soundPosX = pos.x;
		pos.x -= 1.0f * DISTANCEFACTOR;
		if (pos.x < -24 * DISTANCEFACTOR)
		{
			pos.x = -24 * DISTANCEFACTOR;
		}
		mresult = pChannel[11]->set3DAttributes(&pos, &vel);
		errorcheck(mresult);
	}

	//===============================================================================================
	//X Key
	else if ((GetKeyState(0x58) < 0) && !mkeydown)
	{
		FMOD_VECTOR pos, vel;
		mresult = pChannel[11]->get3DAttributes(&pos, &vel);
		errorcheck(mresult);
		pos.x += 1.0f * DISTANCEFACTOR;
		if (pos.x > 23 * DISTANCEFACTOR)
		{
			pos.x = 23 * DISTANCEFACTOR;
		}
		mresult = pChannel[11]->set3DAttributes(&pos, &vel);
		errorcheck(mresult);
	}

	//===============================================================================================
	//< Key
	else if ((GetKeyState(0xBC) < 0) && !mkeydown)
	{
		FMOD_VECTOR userpos, forward, vel, up;
		mresult = msystem->get3DListenerAttributes(0, &userpos, &vel, &forward, &up);
		errorcheck(mresult);
		userpos.x -= 1.0f * DISTANCEFACTOR;
		if (userpos.x < -24 * DISTANCEFACTOR)
		{
			userpos.x = -24 * DISTANCEFACTOR;
		}
		mresult = msystem->set3DListenerAttributes(0, &userpos, &vel, &forward, &up);
		errorcheck(mresult);
	}

	//===============================================================================================
	//> Key
	else if ((GetKeyState(0xBE) < 0) && !mkeydown)
	{
		FMOD_VECTOR userpos, forward, vel, up;
		mresult = msystem->get3DListenerAttributes(0, &userpos, &vel, &forward, &up);
		errorcheck(mresult);
		userpos.x += 1.0f * DISTANCEFACTOR;
		if (userpos.x > 23 * DISTANCEFACTOR)
		{
			userpos.x = 23 * DISTANCEFACTOR;
		}
		mresult = msystem->set3DListenerAttributes(0, &userpos, &vel, &forward, &up);
		errorcheck(mresult);
	}
	
	//===============================================================================================
	//R Key
	else if ((GetKeyState(0x52) < 0) && !mkeydown)
	{
		mresult = msystem->playSound(pSound[1], pChannelGroup[0], false, &pChannel[1]);
		errorcheck(mresult);
	}
	
	//===============================================================================================
	//T Key
	else if ((GetKeyState(0x54) < 0) && !mkeydown)
	{
		mresult = msystem->playSound(pSound[2], pChannelGroup[0], false, &pChannel[2]);
		errorcheck(mresult);
	}

	//===============================================================================================
	//F Key
	else if ((GetKeyState(0x46) < 0) && !mkeydown)
	{
		mresult = msystem->playSound(pSound[4], pChannelGroup[1], false, &pChannel[4]);
		errorcheck(mresult);
	}

	//===============================================================================================
	//G Key
	else if ((GetKeyState(0x47) < 0) && !mkeydown)
	{
		mresult = msystem->playSound(pSound[5], pChannelGroup[1], false, &pChannel[5]);
		errorcheck(mresult);
	}

	//===============================================================================================
	//V Key
	else if ((GetKeyState(0x56) < 0) && !mkeydown)
	{
		mresult = msystem->playSound(pSound[7], pChannelGroup[2], false, &pChannel[7]);
		errorcheck(mresult);
	}

	//===============================================================================================
	//B Key
	else if ((GetKeyState(0x42) < 0) && !mkeydown)
	{
		mresult = msystem->playSound(pSound[8], pChannelGroup[2], false, &pChannel[8]);
		errorcheck(mresult);
	}
	
	//===============================================================================================
	//Number 0
	else if ((GetKeyState(0x30) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 1
	else if ((GetKeyState(0x31) < 0) && !mkeydown) { //Key down
		mkeydown = true;
		//YOUR CODE HERE
		bool bypass;
		mresult = dspLowPass1->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dspLowPass1->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 2
	else if ((GetKeyState(0x32) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE	
		bool bypass;
		mresult = dspHighPass1->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dspHighPass1->setBypass(!bypass);
		errorcheck(mresult);
	
		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 3
	else if ((GetKeyState(0x33) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE		
		bool bypass;
		mresult = dspFlange1->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dspFlange1->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	//Number 4
	else if ((GetKeyState(0x34) < 0) && !mkeydown) { //Key down
		mkeydown = true;
		//YOUR CODE HERE
		bool bypass;
		mresult = dspLowPass2->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dspLowPass2->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 5
	else if ((GetKeyState(0x35) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE	
		bool bypass;
		mresult = dspHighPass2->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dspHighPass2->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 6
	else if ((GetKeyState(0x36) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE		
		bool bypass;
		mresult = dspFlange2->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dspFlange2->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}
	//===============================================================================================
	//Number 7
	else if ((GetKeyState(0x37) < 0) && !mkeydown) { //Key down
		mkeydown = true;
		//YOUR CODE HERE
		bool bypass;
		mresult = dspLowPass3->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dspLowPass3->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 8
	else if ((GetKeyState(0x38) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE	
		bool bypass;
		mresult = dspHighPass3->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dspHighPass3->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}

	//===============================================================================================
	//Number 9
	else if ((GetKeyState(0x39) < 0) && !mkeydown) {
		mkeydown = true;
		//YOUR CODE HERE		
		bool bypass;
		mresult = dspFlange3->getBypass(&bypass);
		errorcheck(mresult);
		mresult = dspFlange3->setBypass(!bypass);
		errorcheck(mresult);

		Sleep(sleep_ms);
		mkeydown = false;
	}
	
}


int main(){	

	init_fmod();
	
	//Get our channel groups

	mresult = msystem->createChannelGroup("group1", &pChannelGroup[0]);
	mresult = msystem->createChannelGroup("group2", &pChannelGroup[1]);
	mresult = msystem->createChannelGroup("group3", &pChannelGroup[2]);

	//Set the distance units. (meters/feet etc).
	mresult = msystem->set3DSettings(1.0, DISTANCEFACTOR, 1.0f);
	errorcheck(mresult);


	//Load all the sounds
	std::ifstream soundFiles("SoundDetails.txt");
	if (!soundFiles.is_open())
		return 0;

	std::string curLine;

	for (int i = 0; i < 12; i++)
	{
		soundFiles >> curLine;
		if (i < 9)
		{
			mresult = msystem->createSound(curLine.c_str(), FMOD_CREATESAMPLE, 0, &pSound[i]);
			errorcheck(mresult);
		}
		else
		{
			mresult = msystem->createSound(curLine.c_str(), FMOD_3D, 0, &pSound[i]);
			errorcheck(mresult);
			mresult = pSound[i]->set3DMinMaxDistance(0.5f * DISTANCEFACTOR, 5000.0f * DISTANCEFACTOR);
			errorcheck(mresult);
		}


		soundFiles >> curLine;
		songNames[i] = curLine;

		if (i > 8)
		{		
			pSound[i]->setMode(FMOD_LOOP_NORMAL);
			mresult = msystem->playSound(pSound[i], 0, false, &pChannel[i]);
			errorcheck(mresult);

		}
	}

	mresult = msystem->playSound(pSound[0], pChannelGroup[0], false, &pChannel[0]);
	errorcheck(mresult);
	mresult = msystem->playSound(pSound[3], pChannelGroup[1], true, &pChannel[3]);
	errorcheck(mresult);
	mresult = msystem->playSound(pSound[6], pChannelGroup[2], true, &pChannel[6]);
	errorcheck(mresult);

	pSound[0]->setMode(FMOD_LOOP_NORMAL);
	pSound[3]->setMode(FMOD_LOOP_NORMAL);
	pSound[6]->setMode(FMOD_LOOP_NORMAL);

	FMOD_VECTOR pos = { 15.0f * DISTANCEFACTOR, 0.0f, 0.0f };
	FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

	mresult = pChannel[9]->set3DAttributes(&pos, &vel);
	errorcheck(mresult);

	pos = { 0.0f * DISTANCEFACTOR, 0.0f, 0.0f };

	mresult = pChannel[10]->set3DAttributes(&pos, &vel);
	errorcheck(mresult);

	pos = { -15.0f * DISTANCEFACTOR, 0.0f, 0.0f };

	mresult = pChannel[11]->set3DAttributes(&pos, &vel);
	errorcheck(mresult);

	//Create DSP effects
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &dspLowPass1);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_HIGHPASS, &dspHighPass1);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_FLANGE, &dspFlange1);
	errorcheck(mresult);

	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &dspLowPass2);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_HIGHPASS, &dspHighPass2);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_FLANGE, &dspFlange2);
	errorcheck(mresult);

	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &dspLowPass3);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_HIGHPASS, &dspHighPass3);
	errorcheck(mresult);
	mresult = msystem->createDSPByType(FMOD_DSP_TYPE_FLANGE, &dspFlange3);
	errorcheck(mresult);

	//Add effects to each channel group.
	mresult = pChannelGroup[0]->addDSP(0, dspLowPass1);
	errorcheck(mresult);
	mresult = pChannelGroup[0]->addDSP(0, dspHighPass1);
	errorcheck(mresult);
	mresult = pChannelGroup[0]->addDSP(0, dspFlange1);
	errorcheck(mresult);

	mresult = pChannelGroup[1]->addDSP(0, dspLowPass2);
	errorcheck(mresult);
	mresult = pChannelGroup[1]->addDSP(0, dspHighPass2);
	errorcheck(mresult);
	mresult = pChannelGroup[1]->addDSP(0, dspFlange2);
	errorcheck(mresult);

	mresult = pChannelGroup[2]->addDSP(0, dspLowPass3);
	errorcheck(mresult);
	mresult = pChannelGroup[2]->addDSP(0, dspHighPass3);
	errorcheck(mresult);
	mresult = pChannelGroup[2]->addDSP(0, dspFlange3);
	errorcheck(mresult);

	//Bypass all effects, this plays the sound with no effects.
	mresult = dspLowPass1->setBypass(true);
	errorcheck(mresult);
	mresult = dspHighPass1->setBypass(true);;
	errorcheck(mresult);
	mresult = dspFlange1->setBypass(true);
	errorcheck(mresult);

	mresult = dspLowPass2->setBypass(true);
	errorcheck(mresult);
	mresult = dspHighPass2->setBypass(true);;
	errorcheck(mresult);
	mresult = dspFlange2->setBypass(true);
	errorcheck(mresult);

	mresult = dspLowPass3->setBypass(true);
	errorcheck(mresult);
	mresult = dspHighPass3->setBypass(true);;
	errorcheck(mresult);
	mresult = dspFlange3->setBypass(true);
	errorcheck(mresult);

	//Set 3D Listener Attributes
	FMOD_VECTOR userpos = { -24.0f, 0.0f, 0.0f };
	FMOD_VECTOR forward = { 0.0f, 0.0f, 1.0f };
	FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };
	FMOD_VECTOR uservel = { 0.0f, 0.0f, 0.0f };

	mresult = msystem->set3DListenerAttributes(0, &userpos, &uservel, &forward, &up);
	errorcheck(mresult);

	bool isPlaying = false;
	unsigned int channel_position = 0;
	unsigned int sound_length = 0;
	float origSoundFreq = 0;
	float channelPitch;
	FMOD_SOUND_TYPE songType;
	FMOD_SOUND_FORMAT songFormat;
	int songChannels;
	int songBits;

	while (!mis_esc)
	{
		//Needed for print_text
		start_text();

		handle_keyboard();

		

		//Important to update msystem
		mresult = msystem->update();
		errorcheck(mresult);

		//Get dsp status
		bool dsplowpass1_bypass;
		bool dsphighpass1_bypass;
		bool dspflange1_bypass;
		bool dsplowpass2_bypass;
		bool dsphighpass2_bypass;
		bool dspflange2_bypass;
		bool dsplowpass3_bypass;
		bool dsphighpass3_bypass;
		bool dspflange3_bypass;

		dspLowPass1->getBypass(&dsplowpass1_bypass);
		dspHighPass1->getBypass(&dsphighpass1_bypass);
		dspFlange1->getBypass(&dspflange1_bypass);
		dspLowPass2->getBypass(&dsplowpass2_bypass);
		dspHighPass2->getBypass(&dsphighpass2_bypass);
		dspFlange2->getBypass(&dspflange2_bypass);
		dspLowPass3->getBypass(&dsplowpass3_bypass);
		dspHighPass3->getBypass(&dsphighpass3_bypass);
		dspFlange3->getBypass(&dspflange3_bypass);


		isPlaying = false;
		pSound[currentChannel]->getDefaults(&origSoundFreq, 0);
		pSound[currentChannel]->getFormat(&songType, &songFormat, &songChannels, &songBits);
		pSound[currentChannel]->getLength(&songLength, FMOD_TIMEUNIT_MS);
		//Get position
		pChannel[currentChannel]->getPosition(&channel_position, FMOD_TIMEUNIT_MS);

		//Get pitch
		pChannel[currentChannel]->getPitch(&channelPitch);

		//Get is paused
		if (currentChannel <= 2)	//Constantly streaming music
			pChannel[currentChannel]->getPaused(&isPlaying);
		else						//Sound effects
		{
			pChannel[currentChannel]->isPlaying(&isPlaying);
			isPlaying = !isPlaying;
		}

		//Get Volume
		pChannel[currentChannel]->getVolume(&mvolume);

		pChannel[currentChannel]->getPosition(&posInSong, FMOD_TIMEUNIT_MS);

		//Get 3D positions
		FMOD_VECTOR soundpos;
		mresult = msystem->get3DListenerAttributes(0, &soundpos, 0, 0, 0);
		errorcheck(mresult);
		char u[80] = "|................................................|";
		u[(int)(soundpos.x / DISTANCEFACTOR) + 25] = 'U';

		mresult = pChannel[9]->get3DAttributes(&soundpos, 0);
		errorcheck(mresult);
		char s1[80] = "|................................................|";
		s1[(int)(soundpos.x / DISTANCEFACTOR) + 25] = '1';

		mresult = pChannel[10]->get3DAttributes(&soundpos, 0);
		errorcheck(mresult);
		char s2[80] = "|................................................|";
		s2[(int)(soundpos.x / DISTANCEFACTOR) + 25] = '2';

		mresult = pChannel[11]->get3DAttributes(&soundpos, 0);
		errorcheck(mresult);
		char s3[80] = "|................................................|";
		s3[(int)(soundpos.x / DISTANCEFACTOR) + 25] = '3';

		print_text("==============================================================");
		print_text("Media Fundamentals... Effects");
		print_text("==============================================================");
		print_text("");
		print_text("Sound Name: %s          ", songNames[currentChannel].c_str());
		print_text("");
		print_text("Group Adjust: [%c]", groupAdjust ? 'x' : ' ');
		print_text("Current Position: %d:%d%d:%d%d out of %d:%d%d:%d%d", posInSong / 60000, (posInSong / 10000) % 6, (posInSong / 1000) % 10, (posInSong / 100) % 10, (posInSong / 10) % 10, songLength / 60000, (songLength / 10000) % 6, (songLength / 1000) % 10, (songLength / 100) % 10, (songLength / 10) % 10);
		print_text("Native Frequency: %f", origSoundFreq);
		print_text("Current Volume: %f", songVolume[currentChannel]);
		print_text("Current Pitch: %f", songPitch[currentChannel]);
		print_text("Pan Balance: %f", songPan[currentChannel]);
		print_text("Sound Type: %s", FMOD_TypeToString(songType));
		print_text("Sound Format: %s", FMOD_FormatToString(songFormat));
		print_text("Number of Channels Used: %d", songChannels);
		print_text("Bits Per Sample: %d", songBits);
		print_text("Press 1 to toggle dsplowpass effect");
		print_text("Press 2 to toggle dsphighpass effect");
		print_text("Press 3 to toggle dspflange effect");		
		print_text("");
		if(currentGroup == 0)
			print_text("Effect status: lowpass[%c] highpass[%c] flange[%c]",			
				dsplowpass1_bypass ? ' ' : 'x',
				dsphighpass1_bypass ? ' ' : 'x',
				dspflange1_bypass ? ' ' : 'x');
		else if (currentGroup == 1)
			print_text("Effect status: lowpass[%c] highpass[%c] flange[%c]",
				dsplowpass2_bypass ? ' ' : 'x',
				dsphighpass2_bypass ? ' ' : 'x',
				dspflange2_bypass ? ' ' : 'x');
		else if (currentGroup == 2)
			print_text("Effect status: lowpass[%c] highpass[%c] flange[%c]",
				dsplowpass3_bypass ? ' ' : 'x',
				dsphighpass3_bypass ? ' ' : 'x',
				dspflange3_bypass ? ' ' : 'x');
		print_text("");
		print_text(u);
		print_text(s1);
		print_text(s2);
		print_text(s3);

		//Needed for print_text
		end_text();
		
		Sleep(INTERFACE_UPDATE_TIME - 1);
	}

	

	//Shutdown
	for (int i = 0; i < NUMBER_OF_SOUNDS; i++)
	{
		if (msounds[i])
		{
			msounds[i]->release();
			errorcheck(mresult);
		}
	}



	if (msystem) {
		mresult = msystem->close();
		errorcheck(mresult);
		mresult = msystem->release();
		errorcheck(mresult);
	}
	

	return 0;
}

void volumeUp(int chan)
{
	if (songVolume[chan] < 5)
	{
		songVolume[chan] += 0.05f;
		pChannel[chan]->setVolume(songVolume[chan]);
	}
	else
	{
		songVolume[chan] = 5;
		pChannel[chan]->setVolume(songVolume[currentChannel]);
	}
}
void volumeDown(int chan)
{
	if (songVolume[chan] > 0)
	{
		songVolume[chan] -= 0.05f;
		pChannel[chan]->setVolume(songVolume[chan]);
	}
	else
	{
		songVolume[chan] = 0;
		pChannel[chan]->setVolume(songVolume[chan]);
	}
}
void panLeft(int chan)
{
	if (songPan[chan] > -1)
	{
		songPan[chan] -= 0.05;
		pChannel[chan]->setPan(songPan[chan]);
	}
	else
	{
		songPan[chan] = -1;
		pChannel[chan]->setPan(songPan[chan]);
	}
}
void panRight(int chan)
{
	if (songPan[chan] < 1)
	{
		songPan[chan] += 0.05;
		pChannel[chan]->setPan(songPan[chan]);
	}
	else
	{
		songPan[chan] = 1;
		pChannel[chan]->setPan(songPan[chan]);
	}
}
void speedUp(int chan)
{
	if (songSpeed[chan] < 10.0f)
	{
		songSpeed[chan] += 0.02f;
		pSound[chan]->setMusicSpeed(songSpeed[chan]);
	}
	else
	{
		songSpeed[chan] = 10.0f;
		pSound[chan]->setMusicSpeed(songSpeed[chan]);
	}
}
void speedDown(int chan)
{
	if (songSpeed[chan] >= 0.03f)
	{
		songSpeed[chan] -= 0.02f;
		pSound[chan]->setMusicSpeed(songSpeed[chan]);
	}
	else
	{
		songSpeed[chan] = 0.01f;
		pSound[chan]->setMusicSpeed(songSpeed[chan]);
	}
}
void pitchUp(int chan)
{
	if (songPitch[chan] < 10.0f)
	{
		songPitch[chan] += 0.02f;
		pChannel[chan]->setPitch(songPitch[chan]);
	}
	else
	{
		songPitch[chan] = 10.0f;
		pChannel[chan]->setPitch(songPitch[chan]);
	}
}
void pitchDown(int chan)
{
	if (songPitch[chan] > 0.03f)
	{
		songPitch[chan] -= 0.02f;
		pChannel[chan]->setPitch(songPitch[chan]);
	}
	else
	{
		songPitch[chan] = 0.01;
		pChannel[chan]->setPitch(songPitch[chan]);
	}
}