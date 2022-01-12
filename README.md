# FufuPiano

Produce single-finger piano videos via MIDI files and video samples.

## How to Use

- Download a release or clone and compile one.
- Put ffmpeg.exe just beside FufuPiano.exe.
- Put your video samples in folder "sample"
  - The name of each sample should be < KeyNumber >.< Video Format >.
  - Define the video format (FORMAT) in FufuPiano.cpp (.mov by default).
  - Add back.< Video Format > as a background video for spare time.
- Drop a MIDI file to FufuPiano.exe to run it.
  - The MIDI file can be multi tracked.
  - There shouldn't be overlapped notes in each track.
  - The overlapped notes in the same track will be ignored.

## How it Works

- Read MIDI notes from MIDI file via midifile library.
- Cut video samples into required length via FFmpeg and save them as a series of video in the sequence of the MIDI notes.
- Combine each video in the sequence via FFmpeg.

## Using Library

- [midifile](https://github.com/craigsapp/midifile)
- [FFmpeg](http://ffmpeg.org/)

## Platform

Win 32

Develop: Visual Studio 2015 or other versions