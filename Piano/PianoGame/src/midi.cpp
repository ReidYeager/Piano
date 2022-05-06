
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <cerrno>
#include <stdint.h>
#include <stdio.h>
#include <unordered_map>
#include <cstring>

#include "defines.h"
#include "piano_hero.h"

int LoadFile(const char* _dir, unsigned char** _buffer)
{
  std::string dir(PIANO_RESOURCE_DIR);
  dir.append("songs/");
  dir.append(_dir);

  std::ifstream myfile(dir.c_str());
  if (!myfile.is_open())
  {
    printf("Failed to open file\n> '%s'\n", dir.c_str());
    return EXIT_FAILURE;
  }

  myfile.seekg(0, std::ios::end);
  int filesize = myfile.tellg();
  myfile.seekg(0, std::ios::beg);

  *_buffer = (unsigned char*)malloc(filesize);
  memset(*_buffer, 0, filesize);

  myfile.read((char*)*_buffer, filesize);

  return filesize;
}

b8 SearchForFileStart(const unsigned char* _input, u32* _index, u32 _length)
{
  while (*_index <= _length - 5)
  {
    if (_input[*_index + 0] == 0x50 &&
        _input[*_index + 1] == 0x69 &&
        _input[*_index + 2] == 0x61 &&
        _input[*_index + 3] == 0x6e &&
        _input[*_index + 4] == 0x6f)
    {
      *_index += 5;
      return true;
    }

    (*_index)++;
  }

  return false;
}

void ReadDeltaTime(const unsigned char* _input, u32& _readIndex, u32& _deltaTime)
{
  _deltaTime = _input[_readIndex] & 0x7f;
  _readIndex++;

  // An active highest bit indicates the time includes the next byte
  while (_input[_readIndex - 1] & 0x80)
  {
    u8 tmpByte = 0;
    tmpByte = _input[_readIndex];
    _readIndex++;

    _deltaTime = (_deltaTime << 7) | tmpByte & 0x7f;
  }
}

int fileSize = 0;

u32 SearchForFileEnd(const unsigned char* _input, u32 _fileSize)
{
  for (u32 i = _fileSize - 3; i > 0; i--)
  {
    if (_input[i] == 255)
    {
      if (_input[i + 1] == 47 && _input[i + 2] == 0)
      {
        return i;
      }
    }
  }

  return 0;
}

std::vector<Piano::note> LoadNotesFromFile(const char* _file)
{
  unsigned char* input = nullptr;
  fileSize = LoadFile(_file, &input);
  printf("Loaded %d bytes from '%s'\n", fileSize, _file);

  std::vector<u32> deltaTimes;
  std::vector<b8> actions;
  std::vector<u8> noteNumbers;
  std::vector<u8> velocities;
  u32 readIndex = 0;
  u32 cycles = 0; // Rename to "noteCount"

  // Determine start position =====
  if (input[37] == 0)
  {
    readIndex = 41;
  }
  else if (input[21] >= (fileSize - 22))
  {
    readIndex = 37;
  }
  else if (!SearchForFileStart(input, &readIndex, fileSize))
  {
    printf("Piano not found\n");
    return {};
  }

  u32 end = SearchForFileEnd(input, fileSize);

  // Read MIDI =====
  u32 note;
  u32 pressed;
  u32 velocity;
  u32 deltaTime;

  while (readIndex < end - 2)
  {
    // Delta time =====
    ReadDeltaTime(input, readIndex, deltaTime);
    deltaTimes.push_back(deltaTime);

    // Pressed =====

    // Inputs copy the action of the previous note
    if (input[readIndex] < 0x80)
    {
      actions.push_back(actions.back());

      // Note =====
      noteNumbers.push_back(input[readIndex]);
      readIndex++;

      // Velocity =====
      velocities.push_back(input[readIndex]);
      readIndex++;
    }
    else
    {
      u8 action = input[readIndex];
      u8 high = action & 0xf0;
      readIndex++;

      if (high == 0x90) // Pressed
      {
        actions.push_back(1);

        // Note =====
        noteNumbers.push_back(input[readIndex]);
        readIndex++;

        // Velocity =====
        velocities.push_back(input[readIndex]);
        readIndex++;
      }
      else if (high == 0x80) // Released
      {
        actions.push_back(0);

        // Note =====
        noteNumbers.push_back(input[readIndex]);
        readIndex++;

        // Velocity =====
        velocities.push_back(input[readIndex]);
        readIndex++;
      }
      else if (high == 0xa0) // Aftertouch
      {
        readIndex += 2; // Skip the next 2 bytes
      }
      else if (high == 0xb0) // Control change
      {
        readIndex += 2; // Skip the next 2 bytes
      }
      else if (high == 0xc0) // Program change
      {
        readIndex += 1; // Skip the next 1 byte
      }
      else if (high == 0xd0) // Channel pressure
      {
        readIndex += 1; // Skip the next 1 byte
      }
      else if (high == 0xe0) // Pitch bend
      {
        readIndex += 2; // Skip the next 2 bytes
      }
      else if (high == 0xf0) // System exclusive
      {

      }
      else
      {
        printf("Unknown action byte %hhu", action);
      }
    }

    // Note complete =====
    cycles++;
  }

  printf("==========\n");
  printf("Notes:\n");
  printf("==========\n");

  struct NoteInfo
  {
    u32 StartTime;
  };

  u32 time = 0;
  std::unordered_map<u32, NoteInfo> activeMap;

  struct vec3
  {
    u32 note;
    u32 start;
    u32 duration;
  };

  std::vector<Piano::note> displayedNotes;

  //printf("Delta -- Action -- Note -- Velocity\n");
  for (u32 i = 0; i < cycles; i++)
  {
    //printf("%2.3f -- %4d -- %4d -- %4d\n",
    //       deltaTimes[i] / 1000.0f,
    //       actions[i],
    //       noteNumbers[i],
    //       velocities[i]);

    time += deltaTimes[i];

    if (actions[i])
    {
      activeMap[noteNumbers[i]] = { time };
    }
    else
    {
      auto note = activeMap.find(noteNumbers[i]);
      if (note != activeMap.end())
      {
        u32 start = activeMap[noteNumbers[i]].StartTime;

        Piano::note newNote{};
        newNote.keyPosition = noteNumbers[i];
        newNote.startTime = start * 0.001f;
        newNote.duration = (time - start) * 0.001f;

        displayedNotes.push_back(newNote);
        activeMap.erase(noteNumbers[i]);
      }
    }

  }

  free(input);

  return displayedNotes;
}
