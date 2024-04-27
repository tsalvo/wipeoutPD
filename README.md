# wipeoutPD

A Playdate port of [wipeout-rewrite](https://github.com/phoboslab/wipeout-rewrite) by [phoboslab](https://github.com/phoboslab)

So far, it is hard-coded to show the attract mode for one ship on a time trial. All graphics are drawn in a simple wireframe mode.

⚠️ Work in progress. Expect bugs.


# Screenshots
![Scenery Off](/Screenshots/wipeoutPD-screen1.png?raw=true) ![Scenery On](/Screenshots/wipeoutPD-screen2.png?raw=true) 


# Assets

This repository does not contain the assets (textures, 3D models etc.) needed to run the game. See [wipeout-rewrite README.md](https://github.com/phoboslab/wipeout-rewrite/blob/master/README.md) for more details.

Assets should be placed in the `/Source` directory, for example:

```
./Source/Track01.wav # optional background music
./Source/wipeout/common/allsh.prm # ships
./Source/wipeout/common/alcol.prm # collision
./Source/wipeout/track06/track.trv # track 06 vertices
./Source/wipeout/track06/track.trf # track 06 faces
./Source/wipeout/track06/track.trs # track 06 sections
./Source/wipeout/track06/scene.prm # track 06 surrounding scenery
```

## Creating an Audio Track
Example: from `input.mp3`
`ffmpeg -i input.mp3 -acodec adpcm_ima_wav Track01.wav`


# Building

## Nova
Use the integrated Playdate Simulator Tasks for building and running

## Linux / Mac OS
Use the `make` command to create a wipeout.pdx bundle. See section on *Assets* above

# License - same as [wipeout-rewrite](https://github.com/phoboslab/wipeout-rewrite)

There is none.

This code can NOT be used to make a commercial release of Wipeout for Playdate. It is intended only for building and running  on your own simulator or device.
See [wipeout-rewrite README.md](https://github.com/phoboslab/wipeout-rewrite/blob/master/README.md) for details

