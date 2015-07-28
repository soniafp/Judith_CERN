#include "kartelconvert.h"

#include <cassert>
#include <iostream>
#include <stdio.h>

#include "../storage/hit.h"
#include "../storage/cluster.h"
#include "../storage/plane.h"
#include "../storage/event.h"
#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
#include "../storage/storageio.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;
using std::fstream;

namespace Converters {

/* Check if a specific bit is non-zero in a word. If it is, this bit is
 * usually then written into the appropriate variable. */
inline bool KartelConvert::checkBit(Word checkWord, unsigned int pos)
{
  if (checkWord & (int)pow(2.0, pos)) return true;
  return false;
}

/* Set the specified bit to 1 in the word */
inline void KartelConvert::setBit(Word& setWord, unsigned int pos)
{
  setWord = setWord | (int)pow(2.0, pos);
}

/* Clear all the frame variables so that a new one can be read */
void KartelConvert::clearFrame()
{
  _frame.timeStamp = 0;
  _frame.lineCount = 0;
  _frame.consecCount = 0;
  _frame.triggerFlag = 0;
  _frame.triggerOffset = 0;

  for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
  {
    _frame.wordCount[nplane] = 0;
    _frame.ch0Header[nplane] = 0;
    _frame.ch1Header[nplane] = 0;
    _frame.number[nplane] = 0;
    _frame.length[nplane] = 0;
    _frame.hits[nplane].clear();
    _frame.planeEnded[nplane] = false;
  }

  _frame.errHeader = false;
  _frame.errEof = false;
  _frame.errSync = false;
  _frame.errOffset = false;
  _frame.errHits = false;
  _frame.errSum = false;
}

/* Clear the line buffer variables so that the next line can be read */
void KartelConvert::clearLineBuffer() {
  _line.sync = 0;
  _line.triggerOffset = 0;
  _line.triggerNumber = 0;
  for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
  {
    _line.ch0[nplane] = 0;
    _line.ch1[nplane] = 0;
  }
}

void KartelConvert::fillBuffer()
{
  // Read the buff_size length of Words into the file_buffer array
  _kartelInput.read((char*)_fileBuffer, _buffSize * sizeof(Word));

  // How many chars were ACTUALLY read (not always specified amount)
  _buffLength = _kartelInput.gcount();

  // Could have stopped reading inside a word, round down
  const unsigned int intLength =
      (int)(_buffLength / sizeof(Word)) * sizeof(Word);
  const unsigned int ndiscard = _buffLength - intLength;
  for (unsigned int i = 0; i < ndiscard; i++) _kartelInput.unget();
  _buffLength = intLength;

  _buffPos = 0;
}

/* Read the next word from the input stream. Returns false if the end of file
 * is reached. */
int KartelConvert::readNextWord()
{
  // If this is the last buffer, and there isn't a word left in it
  if (!_kartelInput.good() && (_buffPos + 1) * sizeof(Word) > _buffLength)
  {
    _eofReached = true;
    return -1;
  }

  if ((_buffPos + 1) * sizeof(Word) > _buffLength)
    fillBuffer();

  _word = _fileBuffer[_buffPos];
  _buffPos++;

  return 0;
}

/* Finds the next synchronization signal in the stream, indicating that a frame
 * is starting. Returns false if it can't fine one. */
int KartelConvert::findNextSync()
{
  // Search over a maximum number of words for the next synch signal
  while (!readNextWord())
  {
    // Check if the current word contains a synchronization signal bit
    if (checkBit(_word, _bitMap.sync))
    {
      _buffPos--; // Seek back to the last word
      return 0; // Found the synchronization signal
    }
  }

  // Reached EOF
  return -1;
}

/* Read the next line from the input stream. Stores the various variable
 * information into the appropriate containers in the line buffer. Returns
 * -2 if there EOF is reached in the line. */
int KartelConvert::readNextLine()
{
  // Reset the line stream
  clearLineBuffer();

  // Read one line of words from the raw data, and map the bits to the stream
  for (unsigned int nbit = 0; nbit < _lineLength; nbit++)
  {
    if (readNextWord()) return -1;

    // Copy the bit from the synch position of the stream to the appropriate line variable
    if (checkBit(_word, _bitMap.sync)) setBit(_line.sync, nbit);

    // Map the bits from each plane's channels
    for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
    {
      if (checkBit(_word, _bitMap.ch0[nplane])) setBit(_line.ch0[nplane], nbit);
      if (checkBit(_word, _bitMap.ch1[nplane])) setBit(_line.ch1[nplane], nbit);
    }

    // The first line of a frame should have a trigger offset
    if (_frame.lineCount == 0)
      if (checkBit(_word, _bitMap.triggerOffset)) setBit(_line.triggerOffset, nbit);

    // Should also have a trigger number
    if (_frame.lineCount == 0)
      if (checkBit(_word, _bitMap.trigger)) setBit(_line.triggerNumber, nbit);

    // The time stamp is written over the first four lines of each frame
    if (_frame.lineCount < 4 && checkBit(_word, _bitMap.timeStamp))
    {
      // convert the current bit to a 64 bit position
      const unsigned int pos = nbit + sizeof(Word) * _frame.lineCount;
      _frame.timeStamp = _frame.timeStamp | (int)pow(2.0, pos);
    }
  }

  _frame.lineCount++;

  return 0;
}

/* Checks if the current line contains a termination signal for one of the
 * planes, indicating that all hits have been read from this plane. */
bool KartelConvert::endOfPlane(int nplane)
{
  if (_line.ch0[nplane] == _footer || _line.ch1[nplane] == _footer)
    return true;
  return false;
}

/* Checks if all the planes have finished reading the hits, indicating that
 * the frame has ended. */
bool KartelConvert::frameEnded() {
  unsigned int numEnded = 0;

  for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
    if (_frame.planeEnded[nplane]) numEnded++;

  if (numEnded == _numPlanes) return true;
  else return false;
}

int KartelConvert::checkHit(const Hit& hit, unsigned int nsensor)
{
  if (_device) assert(nsensor < _device->getNumSensors() &&
                      "KatelConvert: device doesn't have the correct number of planes");
  if (!_device) return 0;
  Mechanics::Sensor* sensor = _device->getSensor(nsensor);
  if (hit.x > sensor->getNumX() || hit.y > sensor->getNumY()) return -1;
  if (sensor->isPixelNoisy(hit.x, hit.y)) return 1;
  return 0;
}

int KartelConvert::readHits()
{
  // Variables to organize the order of reading the hits from rows/cols
  bool readRow[_numPlanes]; // Switch to indicate if we need to read a row or a column
  unsigned int readHits[_numPlanes]; // Read this many hits from the x axis before going back to y

  for (unsigned int i = 0; i < _numPlanes; i++)
  {
    readRow[i] = true;
    readHits[i] = 0;
  }

  // Buffers to hold the hit info until it is written to the frame
  unsigned int row[_numPlanes] = { 0 }; // The row which is hit
  unsigned int col[_numPlanes] = { 0 }; // The first column hit in this row
  unsigned int num[_numPlanes] = { 0 }; // The number of consecutive hits after the indicated column

  // Read lines until the frame ends
  while (!frameEnded())
  {
    if (readNextLine()) {
      _frame.errEof = true;
      return -1;
    }

    // There should be no synch signal in the hit lines
    if (_line.sync) {
      _frame.errSync = true;
      return -1;
    }

    // Serialize the two channels
    for (unsigned int serial = 0; serial < 2; serial++)
    {
      for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
      {
        // Check if the read out of this plane has already ended
        if (_frame.planeEnded[nplane]) continue;

        // Check if this is the end of this plane's read out
        if (endOfPlane(nplane))
        {
          _frame.planeEnded[nplane] = true;
          continue;
        }

        // Get the data to parse
        Word* data = 0; // Pointer to the ch0 or 1 of this plane
        if (!serial) data = &(_line.ch0[nplane]);
        else         data = &(_line.ch1[nplane]);

        // Indicate that a new word has been read in this plane
        _frame.wordCount[nplane]++;

        // Read info from a row
        if (readRow[nplane])
        {
          row[nplane] = (*data >> 4) & 0x07FF; // Gets the number of the next row with hits
          readHits[nplane] = *data & 0x000F; // Gets the number of columns hit in this row
          readRow[nplane] = false; // Indicate to read the columns next time
        }
        // Read info from a column
        else
        {
          col[nplane] = (*data >> 2) & 0x07FF; // Read the column which was hit
          num[nplane] = *data & 0x0003; // Read how many consecutive columns also register a hit

          // Indicate that another of the columns has been read out for this row
          readHits[nplane]--;
          // If all the columns are read in the row, read another row next
          if (readHits[nplane] == 0) readRow[nplane] = true;

          // Check if this hit is inside the sensor
          if (row[nplane] >= _nrows || col[nplane] + num[nplane] >= _ncols)
          {
            _frame.errHits = true;
            return -1;
          }

          for (unsigned int n = 0; n <= num[nplane]; n++)
          {
            Hit hit;
            hit.x = col[nplane] + n;
            hit.y = row[nplane];
            const int code = checkHit(hit, nplane);
            if (code == -1)
            {
              _frame.errHits = true;
              return -1;
            }
            if (!code) _frame.hits[nplane].push_back(hit);
          }
        }
      } // next plane
    } // next channel
  } // next frame

  // Check that the given frame length was read
  for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
  {
    if (_frame.length[nplane] != _frame.wordCount[nplane])
    {
      _frame.errSum = true;
      return -1;
    }
  }

  return 0;
}

/* Reads three lines. If these two lines are at the start of a frame (i.e. there
 * is a sync signal), they will provide info about the frame. */
int KartelConvert::readFrameInfo()
{
  if (readNextLine())
  {
    _frame.errEof = true;
    return -1;
  }

  // Get the trigger and offset
  _frame.triggerFlag = _line.triggerNumber;
  _line.triggerOffset = _line.triggerOffset >> 1; // Matevz' excellent FPGA skills introduced an offset
  _frame.triggerOffset = _line.triggerOffset;

  // Check for problems

  // This line should still contain a synch signal
  if (_line.sync != _sync) {
    _frame.errSync = true;
    return -1;
  }

  // This line should contain the headers
  for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
 {
    _frame.ch0Header[nplane] = _line.ch0[nplane];
    _frame.ch1Header[nplane] = _line.ch1[nplane];

    if (_line.ch0[nplane] != _header || _line.ch1[nplane] != _header)
    {
      _frame.errHeader = true;
      return -1;
    }
  }

  // Trigger offset should be smaller than the number of clock cycles
  if (_frame.triggerOffset >= _clock)
  {
    _frame.errOffset = true;
    return -1;
  }

  if (readNextLine())
  {
    _frame.errEof = true;
    return -1;
  }

  // The next line will give the number of this frame
  for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
  {
    _frame.number[nplane] = _line.ch1[nplane];
    _frame.number[nplane] = _frame.number[nplane] << 16;
    _frame.number[nplane] = _frame.number[nplane] | _line.ch0[nplane];
  }

  if (readNextLine())
  {
    _frame.errEof = true;
    return -1;
  }

  // The next line will give the length of the frame
  for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
    _frame.length[nplane] = _line.ch0[nplane] + _line.ch1[nplane];

  return 0;
}

void KartelConvert::writeFrame(bool invalid)
{
  // Currently, there is no timestamp. Instead, use trigger number and offset
  ULong64_t timeStamp = _frame.number[0] * _clock - _frame.triggerOffset;

  Storage::Event* event = new Storage::Event(_numPlanes);
  event->setTimeStamp(timeStamp);
  event->setFrameNumber(_frame.number[0]);
  event->setTriggerOffset(_frame.triggerOffset);
  event->setTriggerInfo(_frame.consecCount);
  event->setInvalid(invalid);

  if (!invalid)
  {
    for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
    {
      for (unsigned int nhit = 0; nhit < _frame.hits[nplane].size(); nhit++)
      {
        const unsigned int x = _frame.hits[nplane].at(nhit).x;
        const unsigned int y = _frame.hits[nplane].at(nhit).y;
        Storage::Hit* hit = event->newHit(nplane);
        hit->setPix(x, y);
      }
    }
  }

  try
  {
    // If this fails, nothing was added to the file
    _storage->writeEvent(event);
  }
  catch (const char* e)
  {
    if (VERBOSE) cout << e << endl;
    _errWrite++;

    // Write instead an invalid event, if this fails <divinity of choice> help us all
    delete event;
    invalid = true;
    event = new Storage::Event(_numPlanes);
    event->setTimeStamp(timeStamp);
    event->setFrameNumber(_frame.number[0]);
    event->setTriggerOffset(_frame.triggerOffset);
    event->setInvalid(invalid);
    _storage->writeEvent(event);
  }

  _totalEvents++;
  if (invalid) _invalidEvents++;

  // Delete the event, this will also delete the hits
  delete event;
}

int KartelConvert::processEvent(bool discard)
{
  if (!_kartelInput.is_open())
  {
    _eofReached = true;
    return -1;
  }

  unsigned int consecCount = 0; // Count consecutive frames
  unsigned int lastFrameNumber = 0;

  // Seach syncs until a triggered event is found
  while (true)
  {
    if (findNextSync())
    {
      _eofReached = true;
      return -1;
    }

    clearFrame();

    int errorStatus = 0;
    errorStatus = readFrameInfo();
    _totalFrames++;

    if (_frame.number[0] == lastFrameNumber + 1)
      consecCount++;
    else
      consecCount = 0;
    lastFrameNumber = _frame.number[0];
    _frame.consecCount = consecCount;

    if (_frame.triggerFlag && !discard)
    {
      if (errorStatus)
        writeFrame(true);
      else if (readHits())
        writeFrame(true);
      else
        writeFrame();
    }

    if (_frame.errHeader) _errHeader++;
    if (_frame.errHits) _errHits++;
    if (_frame.errSum) _errSum++;
    if (_frame.errSync) _errSync++;
    if (_frame.errOffset) _errOffset++;

    // If this was a triggered frame, read the event
    if (_frame.triggerFlag) break;
  }

  return 0;
}

int KartelConvert::processFile(Long64_t numEvents)
{
  _errHeader = 0;
  _errSync = 0;
  _errHits = 0;
  _errSum = 0;
  _errWrite = 0;
  _invalidEvents = 0;
  _totalEvents = 0;
  _totalFrames = 0;
  _possibleDesync = 0;

  // Discard the first triggered event (junk trigger written by firmware)
  //processEvent(true);

  // Loop until numEvents is reached, if it is negative tries to read entire file
  for (Long64_t nevent = 0; nevent != numEvents; nevent++)
  {
    try
    {
      processEvent();
    }
    catch (const char* e)
    {
      cout << e << endl;
      _possibleDesync++;
    }

    // Display a % of progress if numEvents is specified
    const unsigned int update = 1000;
    if (numEvents > 0)
    {
      if (!(nevent % update) || nevent == numEvents - 1)
        cout << "\rProgress: " << (nevent * 100) / (numEvents - 1) << "%" << std::flush;
    }
    // No progress because the files can be >= 4 GB and can't be indexed on 32 bit machines
    else
    {
      if (!(nevent % update) || nevent == numEvents - 1)
        cout << "\rProgress: " << nevent << std::flush;
    }

    if (_eofReached) break;
  }
  cout << endl;

  if (VERBOSE)
  {
    if (_eofReached)
      cout << "EOF terminated the conversion" << endl;

    cout << "\nProcess file summary:\n"
         << "Errors:\n"
         << "  Header : " << _errHeader << "\n"
         << "  Sync   : " << _errSync << "\n"
         << "  Offset : " << _errOffset << "\n"
         << "  Hits   : " << _errHits << "\n"
         << "  Sum    : " << _errSum << "\n"
         << "  Write  : " << _errWrite << "\n"
         << "Number of events:\n"
         << "  Requested : " << numEvents << "\n"
         << "  Read      : " << _totalEvents << "\n"
         << "  Invalids  : " << _invalidEvents << "\n"
         << "  Missed    : " << _possibleDesync << "\n"
         << "  Frames    : " << _totalFrames << endl;
  }

  return 0;
}

int KartelConvert::streamOutput(unsigned int nlines)
{
  findNextSync();

  for (unsigned int i = 0; i < nlines; i++)
  {
    Word sync = 0;
    Word offset = 0;
    Word ch0[_numPlanes] = { 0 };
    Word ch1[_numPlanes] = { 0 };

    // Read one line of words from the raw data, and map the bits to the output variables
    for (unsigned int nbit = 0; nbit < _lineLength; nbit++)
    {
      if (readNextWord()) return -1;

      if (checkBit(_word, _bitMap.sync)) setBit(sync, nbit);
      if (checkBit(_word, _bitMap.triggerOffset)) setBit(offset, nbit);
      for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
      {
        if (checkBit(_word, _bitMap.ch0[nplane])) setBit(ch0[nplane], nbit);
        if (checkBit(_word, _bitMap.ch1[nplane])) setBit(ch1[nplane], nbit);
      }
    }

    // Output
    printf("% 8d | ", offset);
    printf("%4X | ", sync);
    for (unsigned int nplane = 0; nplane < _numPlanes; nplane++) {
      printf("%4X | ", ch0[nplane]);
      printf("%4X | ", ch1[nplane]);
    }
    printf("\n");
  }

  return 0;
}

bool KartelConvert::fileGood() { return !_eofReached; }

double KartelConvert::fileProgress()
{
  fstream::pos_type current = _kartelInput.tellg();
  return (double)current / _endPos;
}

KartelConvert::KartelConvert(const char* fileName, const char* outputName,
                             const Mechanics::Device* device) :
  _device(device),
  _ncols(1152),
  _nrows(576),
  _lineLength(8 * sizeof(Word)),
  _footer(0xAAAA),
  _sync(0xF),
  _header(0x8001)
{
  // Bytes get swaped by firmware, this is a temporary fix until the firmware is improved

  // Offset the given position by 8 (wrap around if higher)
  _bitMap.sync = 0; _bitMap.sync = _bitMap.sync + 8 - 16 * (_bitMap.sync > 7);
  _bitMap.trigger = 15; _bitMap.trigger = _bitMap.trigger + 8 - 16 * (_bitMap.trigger > 7);
  _bitMap.triggerOffset = 12; _bitMap.triggerOffset = _bitMap.triggerOffset + 8 - 16 * (_bitMap.triggerOffset > 7);
  _bitMap.timeStamp = 3; _bitMap.timeStamp = _bitMap.timeStamp + 8 - 16 * (_bitMap.timeStamp > 7);
  _bitMap.ch0[0] = 5; _bitMap.ch0[0] = _bitMap.ch0[0] + 8 - 16 * (_bitMap.ch0[0] > 7);
  _bitMap.ch1[0] = 6; _bitMap.ch1[0] = _bitMap.ch1[0] + 8 - 16 * (_bitMap.ch1[0] > 7);
  _bitMap.ch0[1] = 1; _bitMap.ch0[1] = _bitMap.ch0[1] + 8 - 16 * (_bitMap.ch0[1] > 7);
  _bitMap.ch1[1] = 2; _bitMap.ch1[1] = _bitMap.ch1[1] + 8 - 16 * (_bitMap.ch1[1] > 7);
  _bitMap.ch0[2] = 9; _bitMap.ch0[2] = _bitMap.ch0[2] + 8 - 16 * (_bitMap.ch0[2] > 7);
  _bitMap.ch1[2] = 10; _bitMap.ch1[2] = _bitMap.ch1[2] + 8 - 16 * (_bitMap.ch1[2] > 7);
  _bitMap.ch0[3] = 4; _bitMap.ch0[3] = _bitMap.ch0[3] + 8 - 16 * (_bitMap.ch0[3] > 7);
  _bitMap.ch1[3] = 8; _bitMap.ch1[3] = _bitMap.ch1[3] + 8 - 16 * (_bitMap.ch1[3] > 7);
  _bitMap.ch0[4] = 13; _bitMap.ch0[4] = _bitMap.ch0[4] + 8 - 16 * (_bitMap.ch0[4] > 7);
  _bitMap.ch1[4] = 14; _bitMap.ch1[4] = _bitMap.ch1[4] + 8 - 16 * (_bitMap.ch1[4] > 7);
  _bitMap.ch0[5] = 7; _bitMap.ch0[5] = _bitMap.ch0[5] + 8 - 16 * (_bitMap.ch0[5] > 7);
  _bitMap.ch1[5] = 11; _bitMap.ch1[5] = _bitMap.ch1[5] + 8 - 16 * (_bitMap.ch1[5] > 7);

  _word = 0;
  _buffLength = 0;
  _buffPos = 0;

  _errHeader = 0;
  _errSync = 0;
  _errOffset = 0;
  _errHits = 0;
  _errSum = 0;
  _errWrite = 0;
  _invalidEvents = 0;
  _totalEvents = 0;
  _totalFrames = 0;
  _possibleDesync = 0;

  if (_device && _device->getNumSensors() != _numPlanes)
    throw "KartelConvert: provided device doesn't have the correct number of planes";

  // Open the specified file
  _kartelInput.open(fileName, std::ios::in | std::ios::binary);

 const unsigned int treeMask = Storage::Flags::CLUSTERS | Storage::Flags::TRACKS;
  _storage = new Storage::StorageIO(outputName, Storage::OUTPUT, _numPlanes,
                                    treeMask);

  _eofReached = false;
  // Can't use this anymore due to unsafe handling of large files (64 bit seek)
  //fstream::pos_type start = raw_input.tellg();
  //raw_input.seekg(0, ios::end);
  //end_pos = raw_input.tellg();
  //raw_input.seekg(start);

  if (!_kartelInput.is_open()) _eofReached = true;
}

KartelConvert::~KartelConvert()
{
  // Close if there is an open file
  if (_kartelInput.is_open()) _kartelInput.close();
  if (_storage) delete _storage;
}

}
