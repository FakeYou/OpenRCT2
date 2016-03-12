/*****************************************************************************
* Copyright (c) 2014 Ted John
* OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
*
* This file is part of OpenRCT2.
*
* OpenRCT2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "../game.h"
#include "../interface/console.h"
#include "../interface/window.h"
#include "../localisation/localisation.h"
#include "../util/util.h"
#include "procedural.h"

enum procedural_state state = PROCEDURAL_START;
int designIndex = 0;

static void open_window_new_ride();
static void select_track_design();
static void place_ride();
static void test_ride();
static void check_ride_ratings();
static void demolish_ride();

void procedural_check() {
	if (gStartProcedural) {
		log_verbose("procedural_check, state: %d", state);
		
		switch(state) {
		case PROCEDURAL_START:
			window_close_all();
			state = PROCEDURAL_OPEN_WINDOW_NEW_RIDE;
			break;
		case PROCEDURAL_OPEN_WINDOW_NEW_RIDE:
			open_window_new_ride();
			break;
		case PROCEDURAL_SELECT_TRACK_DESIGN:
			select_track_design();
			break;
		case PROCEDURAL_PLACE_RIDE:
			place_ride();
			break;
		case PROCEDURAL_TEST_RIDE:
			test_ride();
			break;
		case PROCEDURAL_CHECK_RATINGS:
			check_ride_ratings();
			break;
		case PROCEDURAL_DEMOLISH_RIDE:
			demolish_ride();
			break;
		}
	}
}

static void open_window_new_ride() {
	rct_window *w = window_new_ride_open();

	if (!w) {
		return;
	}

	// select 'roller coasters' tab
	window_event_mouse_down_call(w, 6);
	// click the first ride in the list
	window_event_scroll_mousedown_call(w, 0, 70, 70);

	state = PROCEDURAL_SELECT_TRACK_DESIGN;
}

static void select_track_design() {
	// check to make sure the 'track list' window is open
	rct_window *w = window_bring_to_front_by_class(WC_TRACK_DESIGN_LIST);
	if (!w) {
		return;
	}

	// find the index of the 'wooden-simple-elevation' ride in the track design list
	/*
	int index = 0;
	uint8 *trackDesignItem, *trackDesignList = RCT2_ADDRESS(RCT2_ADDRESS_TRACK_LIST, uint8);

	for (trackDesignItem = trackDesignList; *trackDesignItem != 0; trackDesignItem += 128) {

		//if (strcmp(trackDesignItem, "wooden-simple-elevation.TD6") == 0) {
		//	break;
		//}
		if (designIndex == index) {
			break;
		}

		index++;
	}
	*/

	int index = designIndex++;

	if (index > 30) {
		gStartProcedural = false;
		window_close_all();
		designIndex = 0;
		return;
	}

	// click the track design using x,y based on the index
	int x = 10;
	int y = index * 10 + 15;
	window_event_scroll_mousedown_call(w, 0, x, y);

	state = PROCEDURAL_PLACE_RIDE;
}

static void place_ride() {
	// get the center of the main window
	rct_window *w = window_get_main();
	int x = w->width / 2;
	int y = w->height / 2;

	// check to make sure the 'track place' window is open
	w = window_bring_to_front_by_class(WC_TRACK_DESIGN_PLACE);
	if (!w) {
		return;
	}

	// place the ride at the center of the screen
	window_event_tool_down_call(w, 0, x, y);

	state = PROCEDURAL_TEST_RIDE;
}

static void test_ride() {
	// check to make sure the 'ride' window is open
	rct_window *w = window_ride_main_open(0);
	if (!w) {
		return;
	}

	// click 'test light' button
	window_event_mouse_up_call(w, 24);
	
	// set gamespeed to 'hyperspeed' to test rides faster
	gGameSpeed = 8;

	state = PROCEDURAL_CHECK_RATINGS;
}

static void check_ride_ratings() {
	rct_ride *ride = get_ride(0);

	if (ride->ratings.excitement == -1) {
		return;
	}

	// set gamespeed back to 'normal'
	gGameSpeed = 1;

	char name[128];
	format_string(name, ride->name, &ride->name_arguments);

	console_printf("%s, excitement: %d, intensity %d, nausea %d", name,
		ride->ratings.excitement, ride->ratings.intensity, ride->ratings.nausea);
	log_verbose("%s, excitement: %d, intensity %d, nausea %d", name,
		ride->ratings.excitement, ride->ratings.intensity, ride->ratings.nausea);

	state = PROCEDURAL_DEMOLISH_RIDE;
}

static void demolish_ride() {
	// check to make sure the 'ride' window is open
	rct_window *w = window_ride_main_open(0);
	if (!w) {
		return;
	}

	// click 'garbage can' button
	window_event_mouse_up_call(w, 22);

	// confirm demolishing
	w = window_bring_to_front_by_class(WC_DEMOLISH_RIDE_PROMPT);
	window_event_mouse_up_call(w, 3);

	state = PROCEDURAL_START;
	//gStartProcedural = false;
}