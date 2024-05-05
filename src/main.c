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

#define TARGET_FPS 30.0

static bool draw_scenery = true;

static int update(void* userdata);
static void sceneryCheckboxCallback(void* userdata);
static void playTrack01(PlaydateAPI* pd);
PDMenuItem *optionMenuItem = NULL;
PDMenuItem *drawSceneryMenuItem = NULL;
PlaydateAPI* pd = NULL;
static FilePlayer* fp;

int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit )
	{
		pd = playdate;
		LCDFont* font = pd->graphics->loadFont("wipeoutPD", NULL);
		pd->graphics->setFont(font);
		playdate->system->removeAllMenuItems();
		drawSceneryMenuItem = pd->system->addCheckmarkMenuItem("Scenery", 1, sceneryCheckboxCallback, NULL);
		playTrack01(playdate);
		system_init(playdate, TARGET_FPS);
		playdate->system->setUpdateCallback(update, playdate);
	}
	
	return 0;
}

static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	
	system_update(pd, TARGET_FPS, draw_scenery);
        
	pd->system->drawFPS(0,0);

	return 1;
}

static void playTrack01(PlaydateAPI* playdate) {
	// Play Track01.wav (automatically converted into Track01.pda)
	fp = playdate->sound->fileplayer->newPlayer();
	if (playdate->sound->fileplayer->loadIntoPlayer(fp, "Track01.pda")) {
		playdate->sound->fileplayer->play(fp, 0);
	} else {
		playdate->sound->fileplayer->freePlayer(fp);
	}
}

static void sceneryCheckboxCallback(void* userdata) {
	uint8_t newValue = (uint8_t)(pd->system->getMenuItemValue(drawSceneryMenuItem));
	draw_scenery = newValue != 0;
}
