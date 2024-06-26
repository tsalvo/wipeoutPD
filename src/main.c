//
//  main.c
//  Extension
//
//  Created by Dave Hayden on 7/30/14.
//  Copyright (c) 2014 Panic, Inc. All rights reserved.
//

#include <stdint.h>
#include <stdio.h>

#include "pd_api.h"
#include "system.h"

#define TARGET_FPS 30.0F
#define MAX_MUSIC_TRACKS 5
#define TRACK_NAME_LENGTH 12

PlaydateAPI* pd = NULL;

static FilePlayer* fp;
static int numMusicTracks = 0;
static int currentMusicTrack = -1;

static int update(void* userdata);
static void nextSongMenuItemCallback(void* userdata);
static int determineNumberOfTracks(PlaydateAPI* playdate);
static void playNextTrack(PlaydateAPI* pd);
static void trackFinishedCallback(SoundSource* c, void* userdata);

static const char musicTrackNames[MAX_MUSIC_TRACKS][TRACK_NAME_LENGTH] = {
	"Track01.pda\0",
	"Track02.pda\0",
	"Track03.pda\0",
	"Track04.pda\0",
	"Track05.pda\0"
 };

int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit )
	{
		pd = playdate;
		
		LCDFont* font = pd->graphics->loadFont("wipeoutPD", NULL);
		pd->graphics->setFont(font);
		
		numMusicTracks = determineNumberOfTracks(playdate);
		pd->system->logToConsole("numMusicTracks %d", numMusicTracks);
		
		playdate->system->removeAllMenuItems();
		if (numMusicTracks > 1) {
			PDMenuItem* songMenuItem = pd->system->addMenuItem("next song", nextSongMenuItemCallback, NULL);
		}		
		
		playNextTrack(playdate);
		
		system_init(playdate, TARGET_FPS);
		playdate->system->setUpdateCallback(update, playdate);
	}
	
	return 0;
}

static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	
	system_update(pd, TARGET_FPS);
        
	// pd->system->drawFPS(0,0);

	return 1;
}

static int determineNumberOfTracks(PlaydateAPI* playdate) {
	int numTracks = 0;
	for (int i = 0; i < MAX_MUSIC_TRACKS; i++) {
		pd->system->logToConsole("determineNumberOfTracks %d %s", i, musicTrackNames[i]);
		int checkTrack = playdate->file->stat(musicTrackNames[i], NULL);
		if (checkTrack == -1) {
			pd->system->logToConsole("determineNumberOfTracks %s not found", musicTrackNames[i]);
			break;
		}
		numTracks++;
	}
	
	return numTracks;
}

void playNextTrack(PlaydateAPI* playdate) {
	if (numMusicTracks == 0) {
		return;
	} else if (fp == NULL) {
		fp = playdate->sound->fileplayer->newPlayer();
	}
	
	currentMusicTrack += 1;
	currentMusicTrack %= numMusicTracks;
	
	pd->system->logToConsole("playNextTrack %d", currentMusicTrack);
	
	if (playdate->sound->fileplayer->loadIntoPlayer(fp, musicTrackNames[currentMusicTrack])) {
		pd->system->logToConsole("playNextTrack loaded track %d %s", currentMusicTrack, musicTrackNames[currentMusicTrack]);
		playdate->sound->fileplayer->play(fp, numMusicTracks == 1 ? 0 : 1);
		if (numMusicTracks > 1) {
			pd->system->logToConsole("playNextTrack setting up callback");
			playdate->sound->fileplayer->setFinishCallback(fp, trackFinishedCallback, NULL);
		}
	} else {
		playdate->sound->fileplayer->freePlayer(fp);
	}
}

static void nextSongMenuItemCallback(void* userdata) {
	if (fp != NULL) {
		pd->sound->fileplayer->freePlayer(fp);
	}
	fp = NULL;
	
	playNextTrack(pd);
}

static void trackFinishedCallback(SoundSource* c, void* userdata) {
	pd->system->logToConsole("trackFinishedCallback called");
	playNextTrack(pd);
}
