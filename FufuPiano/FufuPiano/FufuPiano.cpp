// FufuPiano.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <conio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <Windows.h>
#include "MidiFile.h"
#include "Options.h"

using namespace std;
using namespace smf;//midifile

#define MAXQUEUELENGTH 1024
#define TIMETHRESHOLD 100
#define FORMAT ".mov"

struct NOTEEVENT {
	uchar note;
	double inTime;
	double outTime;
	
};
NOTEEVENT eventList[MAXQUEUELENGTH];


//add notes to note list
void addNote(NOTEEVENT noteList[MAXQUEUELENGTH], int notePtr, MidiEvent midiEvent, MidiFile m) {
	
	
	noteList[notePtr].note = midiEvent.getKeyNumber();
	noteList[notePtr].inTime = m.getTimeInSeconds(midiEvent.tick);
	
	
}

//write end time of notes in note list
bool endNote(NOTEEVENT noteList[MAXQUEUELENGTH], int &notePtr, MidiEvent midiEvent, MidiFile m) {

	
	if (midiEvent.getKeyNumber() == noteList[notePtr].note) {
		noteList[notePtr].outTime = m.getTimeInSeconds(midiEvent.tick);
		notePtr++;
		return true;
	}
	else {
		//do not move to next note and disable this note close message if this note close message do not match the currently pressed note
		cout << "\n* Notes overlapped.\n\n";
		return false;
	}
	return false;
}

//print note list to console
void printNoteList(NOTEEVENT noteList[MAXQUEUELENGTH], int notePtr) {

	for (int i = 0; i < notePtr; ++i) {
		cout << "#" << i + 1 << "	key=" << (int)noteList[i].note << "	inTime=" << noteList[i].inTime << "	outTime=" << noteList[i].outTime << "\n";
	}
}

void outputVideo(NOTEEVENT noteList[MAXQUEUELENGTH], int numNote, int trackSeq) {


	if (numNote == 0) {
		return;
	}
	string command;
	int seq = 0;
	//cut videos into pieces and sort them
	for (int i = 0; i < numNote; ++i) {
		command = "ffmpeg -ss 0.000 -to " + to_string((double(noteList[i].outTime - noteList[i].inTime)))
			+ " -i sample\\" + to_string(noteList[i].note)
			+ FORMAT + " -c copy temp\\" + to_string(seq) + FORMAT + " -y";
		system(command.c_str());
		seq++;
		//if spare time is too long, add a background video to it
		if (i < numNote - 1 && noteList[i+1].inTime - noteList[i].outTime > (double)TIMETHRESHOLD / 1000) {
			command = "ffmpeg -ss 0.000 -to " + to_string((double(noteList[i + 1].outTime - noteList[i].inTime)))
				+ " -i sample\\back" + FORMAT + " -c copy temp\\" + to_string(seq) + FORMAT + " -y";
			system(command.c_str());
			seq++;
		}
	}
	//write video list
	ofstream ofs("temp\\fileList.txt");
	if (!ofs) {
		cout << "\n* Fail to create file list.\n\n";
		return;
	}
	for (int i = 0; i < seq; ++i) {
		ofs << "file '" + to_string(i) + ".mov'\n";
	}
	ofs.close();
	Sleep(50);
	//conbine video pieces
	command = "ffmpeg -f concat -i temp\\fileList.txt -c copy output\\Track" + to_string(trackSeq) + ".mov";
	system(command.c_str());
}

void printHelp() {
	cout << "Help:\n\nNecessary Files:\n	sample\\<num>.<FORMAT>\n	ffmpeg.exe\n\n";
	cout << "How to Use:\n	Drop a MIDI file on the program.\n	The MIDI file can be multi tracked,\n	but there shouldn't be any overlapped notes in the same track.\n	Type y or N when needing confirmation.\n\n";
	cout << "Learn More:\n	Download source code from github: https://github.com/esun-z \n\n";
}


int main(int argc, char** argv){

	if (strcmp(*argv, "-h")) {
		printHelp();
		return 0;
	}

	//process parameters
	Options options;
	options.process(argc, argv);
	if (options.getArgCount() != 1) {
		cerr << "* Error: Drop a MIDI file to the program to run it correctly.\n";
		_getch();
		exit(1);
	}

	//open midi file
	MidiFile midiFile;
	midiFile.read(options.getArg(1));
	if (!midiFile.status()) {
		cerr << "* Error: Error reading MIDI file: " << options.getArg(1) << "\n";
		_getch();
		exit(1);
	}
	midiFile.absoluteTicks();

	//count truck
	int numTrack = midiFile.getTrackCount();
	cout << numTrack << " trucks found.\n";

	//create path
	system("md sample");
	system("md temp");
	system("md output");

	//initialize
	int eventPtr = 0;
	bool trackOccupied = false;

	//main processer
	for (int cTrack = 0; cTrack < numTrack; ++cTrack) {
		eventPtr = 0;
		trackOccupied = false;
		//read track messages
		cout << "\nTruck #" << cTrack + 1 << ":\n";
		for (int cNote = 0; cNote < midiFile[cTrack].size(); ++cNote) {
			if (midiFile[cTrack][cNote].isNoteOn()) {
				cout << "On: " << midiFile[cTrack][cNote].tick << "	" << midiFile[cTrack][cNote].getKeyNumber() << "\n";
				if (!trackOccupied) {
					trackOccupied = true;
					addNote(eventList, eventPtr, midiFile[cTrack][cNote], midiFile);
				}
			}
			if (midiFile[cTrack][cNote].isNoteOff()) {
				cout << "Off: " << midiFile[cTrack][cNote].tick << "	" << midiFile[cTrack][cNote].getKeyNumber() << "\n";
				if (trackOccupied) {
					if (endNote(eventList, eventPtr, midiFile[cTrack][cNote], midiFile)) {
						trackOccupied = false;
					}
					
				}
			}
		}
		cout << "\nNote List:\n";
		//print track notes
		printNoteList(eventList, eventPtr);
		
		//process video
		cout << "Process this track? (y/n)\n";
		string input;
		cin >> input;
		if (input == "y") {
			outputVideo(eventList, eventPtr, cTrack);
		}
		else {
			cout << "Track process cancelled.\n";
		}
	}

	//clean up
	system("del /q temp");
	system("rmdir temp");

	_getch();

    return 0;
}