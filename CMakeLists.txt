cmake_minimum_required(VERSION 3.14)
set(CMAKE_C_STANDARD 11)

set(ENVSDK $ENV{PLAYDATE_SDK_PATH})

if (NOT ${ENVSDK} STREQUAL "")
	# Convert path from Windows
	file(TO_CMAKE_PATH ${ENVSDK} SDK)
else()
	execute_process(
			COMMAND bash -c "egrep '^\\s*SDKRoot' $HOME/.Playdate/config"
			COMMAND head -n 1
			COMMAND cut -c9-
			OUTPUT_VARIABLE SDK
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()

if (NOT EXISTS ${SDK})
	message(FATAL_ERROR "SDK Path not found; set ENV value PLAYDATE_SDK_PATH")
	return()
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

# Game Name Customization
set(PLAYDATE_GAME_NAME wipeout)
set(PLAYDATE_GAME_DEVICE wipeout_DEVICE)

project(${PLAYDATE_GAME_NAME} C ASM)

if (TOOLCHAIN STREQUAL "armgcc")
	add_executable(${PLAYDATE_GAME_DEVICE} ${SDK}/C_API/buildsupport/setup.c src/main.c src/utils.c src/mem.c src/types.c src/platform.c src/render.c src/system.c src/wipeout/game.c src/wipeout/camera.c src/wipeout/droid.c src/wipeout/object.c src/wipeout/particle.c src/wipeout/race.c src/wipeout/scene.c src/wipeout/sfx.c src/wipeout/ship_ai.c src/wipeout/ship_player.c src/wipeout/ship.c src/wipeout/track.c src/wipeout/weapon.c)
else()
	add_library(${PLAYDATE_GAME_NAME} SHARED src/main.c src/utils.c src/mem.c src/types.c src/platform.c src/render.c src/system.c src/wipeout/game.c src/wipeout/camera.c src/wipeout/droid.c src/wipeout/object.c src/wipeout/particle.c src/wipeout/race.c src/wipeout/scene.c src/wipeout/sfx.c src/wipeout/ship_ai.c src/wipeout/ship_player.c src/wipeout/ship.c src/wipeout/track.c src/wipeout/weapon.c)
endif()

include(${SDK}/C_API/buildsupport/playdate_game.cmake)

