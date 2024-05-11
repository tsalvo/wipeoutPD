# wipeoutPD

A Playdate port of [wipeout-rewrite](https://github.com/phoboslab/wipeout-rewrite) by [phoboslab](https://github.com/phoboslab)

All graphics are drawn in a simple wireframe mode.

⚠️ Work in progress. Expect bugs.


# Screenshots

![Time Trial](/Screenshots/wipeoutPD-timetrial.gif?raw=true) ![Menus](/Screenshots/wipeoutPD-menus.gif?raw=true)


# Assets

This repository does not contain the assets (3D models, collision maps, etc.) needed to run the game. See [wipeout-rewrite README.md](https://github.com/phoboslab/wipeout-rewrite/blob/master/README.md#running) for more details.

Assets should be placed in the `/Source` directory, for example:

```
./Source/Track01.wav # optional background music 1
./Source/Track02.wav # optional background music 2
./Source/Track03.wav # optional background music 3
./Source/Track04.wav # optional background music 4
./Source/Track05.wav # optional background music 5
./Source/wipeout/common/allsh.prm # ships
./Source/wipeout/common/alcol.prm # collision
./Source/wipeout/track01/track.trv # track 01 vertices
./Source/wipeout/track01/track.trf # track 01 faces
./Source/wipeout/track01/track.trs # track 01 sections
./Source/wipeout/track01/scene.prm # track 01 surrounding scenery
./Source/wipeout/track02/track.trv # track 02 vertices
./Source/wipeout/track02/track.trf # track 02 faces
./Source/wipeout/track02/track.trs # track 02 sections
./Source/wipeout/track02/scene.prm # track 02 surrounding scenery
... # (repeat for all tracks)
```

## Creating an Audio Track
Example: from `input.mp3`

`ffmpeg -i input.mp3 -acodec adpcm_ima_wav Track01.wav`


# Building

Install Playdate SDK

## Nova
Use the integrated Playdate Simulator Tasks for building and running

## Linux / Mac OS
Use the `make` command to create a wipeout.pdx bundle. See section on *Assets* above

# License - same as [wipeout-rewrite](https://github.com/phoboslab/wipeout-rewrite/blob/master/README.md#license)

There is none.

This code can NOT be used to make a release of Wipeout for Playdate. It is intended only for building and running on your own simulator or device.
See [wipeout-rewrite README.md](https://github.com/phoboslab/wipeout-rewrite/blob/master/README.md#license) for details

