PRODUCT = wipeout.pdx

# Locate the SDK
SDK = ${PLAYDATE_SDK_PATH}
ifeq ($(SDK),)
	SDK = $(shell egrep '^\s*SDKRoot' ~/.Playdate/config | head -n 1 | cut -c9-)
endif

ifeq ($(SDK),)
$(error SDK path not found; set ENV value PLAYDATE_SDK_PATH)
endif

######
# IMPORTANT: You must add your source folders to VPATH for make to find them
# ex: VPATH += src1:src2
######

VPATH += src

# List C source files here
SRC = src/input.c src/main.c src/utils.c src/mem.c src/types.c src/platform.c src/render.c src/system.c src/wipeout/game.c src/wipeout/camera.c src/wipeout/droid.c src/wipeout/menu.c src/wipeout/main_menu.c src/wipeout/object.c src/wipeout/particle.c src/wipeout/race.c src/wipeout/scene.c src/wipeout/sfx.c src/wipeout/ship_ai.c src/wipeout/ship_player.c src/wipeout/ship.c src/wipeout/title.c src/wipeout/track.c src/wipeout/ui.c src/wipeout/weapon.c

ASRC = setup.s

# List all user directories here
UINCDIR = 

# List user asm files
UASRC = 

# List all user C define here, like -D_DEBUG=1
UDEFS = 

# Define ASM defines here
UADEFS = 

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

include $(SDK)/C_API/buildsupport/common.mk

