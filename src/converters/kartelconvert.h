#ifndef KARTELCONVERT_H
#define KARTELCONVERT_H

#include <fstream>
#include <vector>
#include <iostream>
#include <math.h>

#include <Rtypes.h>
#include <TDirectory.h>

namespace Storage { class StorageIO; }
namespace Mechanics { class Device; }

namespace Converters {

/*******************************************************************************
 * Word in the memory-|
 *                    |
 * LINE: 0000 0000 0000 0000 <-- Word in the file
 *       1111 1111 1111 1111
 *             ... X16
 *       1111 0000 1010 0101
 *
 * Words read in file: 0000 0000 0000 0000, 1111 1111 1111 1111, ...
 * Words read into the memory: 01...1, 01...0, ...
 *
 * This line is comprised of 16 variables, each with 16 bits. The variables
 * are the columns, the values are read down.
 *
 ******************************************************************************/

class KartelConvert
{
public:
  // Defines the size of the base "word" unit in the data
  typedef unsigned short Word;

private:
  static const unsigned int _numPlanes = 6;
  static const unsigned int _buffSize = 4096;
  static const unsigned int _clock = 9216;

  struct Hit {
    unsigned int x;
    unsigned int y;
  };

  struct FrameBuffer {
    ULong64_t timeStamp;
    unsigned int consecCount;
    unsigned int lineCount;
    unsigned int wordCount[_numPlanes];
    Word ch0Header[_numPlanes];
    Word ch1Header[_numPlanes];
    unsigned int number[_numPlanes];
    unsigned int length[_numPlanes];
    unsigned int triggerFlag;
    unsigned int triggerOffset;
    std::vector<Hit> hits[_numPlanes];

    bool planeEnded[_numPlanes];
    bool errHeader; // Incorrect header
    bool errEof;    // End of file reached in frame
    bool errSync;   // Unexpected synch in frame
    bool errOffset; // Error with the trigger offset
    bool errHits;   // Hit value is non sense
    bool errSum;    // Checksum error
  };

  // Gives the bit position within a line of words for the various information
  struct BitPos {
    unsigned int sync;
    unsigned int trigger;
    unsigned int triggerOffset;
    unsigned int timeStamp;
    unsigned int ch0[_numPlanes];
    unsigned int ch1[_numPlanes];
  };

  // Contains information for one line of words in the stream
  struct LineBuffer {
    Word sync;
    Word triggerOffset;
    Word triggerNumber;
    Word ch0[_numPlanes];
    Word ch1[_numPlanes];
  };

  // The inputs and outputs
  std::fstream _kartelInput;
  Storage::StorageIO* _storage;
  const Mechanics::Device* _device;

  // Buffer for the input
  unsigned int _buffLength;
  unsigned int _buffPos; // The position of the current word in the buffer
  Word _fileBuffer[_buffSize];

  const unsigned int _ncols;
  const unsigned int _nrows;
  const unsigned int _lineLength; // Number of words to make one line
  const Word _footer;    // The signal to terminate the hits read from a plane
  const Word _sync;   // The value of the word read from the synch bit positions
  const Word _header; // The value of the headers before hit information

  bool _eofReached;
  std::fstream::pos_type _endPos;

  BitPos      _bitMap; // Position of bits holding information for the various stream components
  LineBuffer  _line;   // Stores the value of the stream components read in one line
  Word        _word;   // Holds one word read from the raw data
  FrameBuffer _frame;  // Variable to hold the parsed stream information

  void fillBuffer();
  inline bool checkBit(Word checkWord, unsigned int pos);
  inline void setBit(Word& setWord, unsigned int pos);
  int readNextWord();
  void clearLineBuffer();
  int readNextLine();
  void clearFrame();
  int findNextSync();
  bool endOfPlane(int nplane);
  bool frameEnded();
  int checkHit(const Hit& hit, unsigned int nsensor);
  int readHits();
  int readFrameInfo();
  void writeFrame(bool invalid = false);

public:
  ULong64_t _errHeader;
  ULong64_t _errSync;
  ULong64_t _errOffset;
  ULong64_t _errHits;
  ULong64_t _errSum;
  ULong64_t _errWrite;
  ULong64_t _invalidEvents;
  ULong64_t _totalEvents;
  ULong64_t _totalFrames;
  ULong64_t _possibleDesync;

  KartelConvert(const char* inputName, const char* outputName,
                const Mechanics::Device* device = 0);
  ~KartelConvert();

  int processEvent(bool discard = false);
  int processFile(Long64_t numEvents = -1);
  int streamOutput(unsigned int nlines);
  bool fileGood();
  double fileProgress();
};

}

#endif // KARTELCONVERT_H
