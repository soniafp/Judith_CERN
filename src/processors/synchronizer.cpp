#include "synchronizer.h"

#include <cassert>
#include <math.h>
#include <iostream>

#include <Rtypes.h>

#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
#include "../storage/event.h"

namespace Processors {

/* Takes an offset from the current _pos (0 is _pos), and returns the index
 * of the buffer element at that offset. Wraps around if necessary */
unsigned int Synchronizer::offsetToIndex(int offset) const
{
  // Going backwards by 1 is the same as going forward by buffer size - 1
  if (offset < 0) offset = _buffSize + offset;

  offset -= (int)(offset / _buffSize) * _buffSize;
  if (_pos + offset >= _buffSize)
    offset -= _buffSize - _pos;
  else
    offset += _pos;

  return offset;
}

void Synchronizer::processEvent(Storage::Event* refEvent,
                                Storage::Event* dutEvent)
{
  ULong64_t lastRef = 0;
  ULong64_t lastDut = 0;

  if (_bufferEmpty)
  {
    _bufferEmpty = false;
    lastRef = refEvent->getTimeStamp();
    lastDut = dutEvent->getTimeStamp();
  }
  // If this isn't the first element being filled
  else
  {
    lastRef = _refEventBuff[_pos]->getTimeStamp();
    lastDut = _dutEventBuff[_pos]->getTimeStamp();
    _pos = offsetToIndex(1); // Increment _pos by 1
    if (_pos == 0) _bufferFull = true;
  }
  //std::cout << "_diffBuff[_pos]: " << _diffBuff[_pos] << std::endl;
  // Remove the element being overwritten from the buffer
  if (_diffBuff[_pos] > _threshold)
    _numOverThreshold--;

  // Normalize the differences to the size of the reference read out window
  const ULong64_t refClockDiff = (refEvent->getTimeStamp() - lastRef);
  const double refFrameDiff = refClockDiff /
      (double)_refDevice->getReadOutWindow();

  const ULong64_t dutClockDiff = (dutEvent->getTimeStamp() - lastDut);
  const double dutFrameDiff = dutClockDiff * _dutDevice->getSyncRatio() /
      (double)_refDevice->getReadOutWindow();

  // Overwrite the current position
  if (_refEventBuff[_pos]) delete _refEventBuff[_pos];
  _refEventBuff[_pos] = 0;
  if (_dutEventBuff[_pos]) delete _dutEventBuff[_pos];
  _dutEventBuff[_pos] = 0;
  _refEventBuff[_pos] = refEvent;
  _dutEventBuff[_pos] = dutEvent;
  _refDiffBuff[_pos] = refFrameDiff;
  _dutDiffBuff[_pos] = dutFrameDiff;
  _diffBuff[_pos] = (fabs(refFrameDiff - dutFrameDiff) /
                     (refFrameDiff + dutFrameDiff)/2.);

  if (_diffBuff[_pos] > _threshold)
    _numOverThreshold++;
}


bool Synchronizer::calculateOffset(unsigned int& desync, unsigned int& refOffset,
                                   unsigned int& dutOffset) const
{
  if (!getFull()) throw "Synchronizer: can't calcualte offset without full buffers";

  refOffset = 0;
  dutOffset = 0;

  const int offset = getOffsetToDesync();
  if (offset < 0) return true; // No desync no need to delete anything
  desync = offset;

  /* Sample case: REF is offset for two, DUT needs to have two events deleted
   * and will recieve an offset of 2 in reading nevent
   *
   * DUT  : 6 7 2 3 4 5  -->  2 3 4 5 6 7
   * REF  : 8 9 2 3 X 7  -->  2 3     X 7 8
   * DIFF : Y Y N N Y Y  : 3 over threshold
   *          ~     ^
   * ~ : pos
   * ^ : desync
   *
   * NOTE: X is because the timestamp difference will be random for the desync
   *       event. This event will never be below threshold.
   *
   * desync = 2 : The desync happens 2 away from the first event in the buffer (pos + 1)
   * Desync happened 6 - 2 = 4 events ago in the buffer from the first event
   *
   * Check only for events from the desync to pos
   *
   * Offset REF by 1 and make new diff
   * 6 7 2 3 4 5
   * 7 8 2 3   X
   * Y Y       Y : 2 diffs
   *
   * Offset DUT by 1 and make new diff
   * 5 6 2 3   4
   * 8 9 2 3 X 7
   * Y Y       Y : 2 diffs
   *
   * Offset REF by 2 and make new diff
   * 6 7 2 3 4 5
   * X 7 2 3
   * Y N         : 1 diffs
   *
   * Run out of information if you offset by 3 you will only see the X desync.
   *
   * If you have A desyncs to play with (4 in this case), and you try an offset
   * of size B, you will be able to check C of the desyncs for agreement, and
   * one will always disagree. So N gives the number of events which will be
   * usable:
   *
   * C = A - B
   *
   * You can check C events, but one will always fail so
   *
   * N = C - 1 = A - B - 1
   *
   * And so the largest offset B you can check such that N = 1
   *
   * B = A - 2
   */

  const unsigned int numDesyncsInBuffer = _buffSize - desync;

  if (numDesyncsInBuffer <= 2)
    throw "Synchronizer: the buffer size is too small to check for any offsets";

  const unsigned int numSyncs = numDesyncsInBuffer - 2;
  // Try to offset the DUT then REF buffer by nsync at a time
  for (unsigned int nsync = 1; nsync <= numSyncs; nsync++)
  {
    // Select the DUT (select) or REF (!select) to offset
    for (unsigned int select = 0; select < 2; select++)
    {
      // Number of offsets still present after this attempt to sync
      unsigned int numOffsets = 0;
      // Number of events which can be shifted at this offset
      const unsigned int testOffsets = numDesyncsInBuffer - nsync;

      assert(testOffsets > 1
             && "Synchronizer: shouldn't be able to check a single event for sync");

      // Add offsets up to and including pos
      for (unsigned int i = 1; i <= testOffsets; i++)
      {
        // One starts at desync, the other starts at desync and is offset by sync
        const unsigned int refIndex = select ? offsetToIndex(desync + i) :
                                               offsetToIndex(desync + nsync + i);
        const unsigned int dutIndex = select ? offsetToIndex(desync + nsync + i) :
                                               offsetToIndex(desync + i);
        const double newDiff = fabs(_refDiffBuff[refIndex] - _dutDiffBuff[dutIndex]);
        if (newDiff > _threshold) numOffsets++;
      }

      // There should be one offset since the offset event used different timestamps
      // and can never be synchronized
      if (numOffsets <= 1)
      {
        refOffset = select ? 0 : nsync;
        dutOffset = select ? nsync : 0;
        return true;
      }
    }
  }

  // If the program gets here, no sync was found (offset is back to 0)
  return false;
}

void Synchronizer::clearBuffers()
{
  _bufferEmpty = true;
  _bufferFull = false;
  _numOverThreshold = 0;
  _pos = 0;
  for (unsigned int i = 0; i < _buffSize; i++)
  {
    if (_refEventBuff[i]) { delete _refEventBuff[i]; }
    if (_dutEventBuff[i]) { delete _dutEventBuff[i]; }
  }
  for (unsigned int i = 0; i < _buffSize; i++)
  {
    _refEventBuff[i] = 0;
    _dutEventBuff[i] = 0;
    _refDiffBuff[i] = 0;
    _dutDiffBuff[i] = 0;
    _diffBuff[i] = 0;
  }
}

Storage::Event*  Synchronizer::getRefTargetEvent() const
{
  // The target event is always the next in the buffer
  unsigned int index = offsetToIndex(1);
  return _refEventBuff[index];
}

Storage::Event*  Synchronizer::getDutTargetEvent() const
{
  unsigned int index = offsetToIndex(1);
  return _dutEventBuff[index];
}

double  Synchronizer::getTargetDiff() const
{
  unsigned int index = offsetToIndex(1);
  return _diffBuff[index];
}

int Synchronizer::getOffsetToDesync() const
{
  for (unsigned int i = 0; i < _buffSize; i++)
  {
    const unsigned int index = offsetToIndex(i + 1); // + 1 gives offset from first event
    if (_diffBuff[index] > _threshold)
      return i;
  }

  return -1; // If no desync is found
}

unsigned int Synchronizer::getNumDiff() const { return _numOverThreshold; }
unsigned int Synchronizer::getSize() const { return _buffSize; }
bool Synchronizer::getFull() const { return _bufferFull; }

Synchronizer::Synchronizer(const Mechanics::Device* refDevice,
                           const Mechanics::Device* dutDevice,
                           double threshold, unsigned int buffSize) :
  _refDevice(refDevice), _dutDevice(dutDevice), _threshold(threshold),
  _buffSize(buffSize), _pos(0), _bufferEmpty(true),
  _bufferFull(false), _numOverThreshold(0)
{
  assert(refDevice && dutDevice &&
         "Synchronization: can't initialize without a reference device");

  _refEventBuff = new Storage::Event*[_buffSize];
  _dutEventBuff = new Storage::Event*[_buffSize];
  _refDiffBuff = new double[_buffSize];
  _dutDiffBuff = new double[_buffSize];
  _diffBuff = new double[_buffSize];

  for (unsigned int i = 0; i < _buffSize; i++)
  {
    _refEventBuff[i] = 0;
    _dutEventBuff[i] = 0;
    _refDiffBuff[i] = 0;
    _dutDiffBuff[i] = 0;
    _diffBuff[i] = 0;
  }
}

Synchronizer::~Synchronizer()
{
  clearBuffers(); // deletes any events in the buffers
  delete[] _refEventBuff;
  delete[] _dutEventBuff;
  delete[] _refDiffBuff;
  delete[] _dutDiffBuff;
  delete[] _diffBuff;
}

}
