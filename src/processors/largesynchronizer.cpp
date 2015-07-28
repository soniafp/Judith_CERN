#include "largesynchronizer.h"

#include <cassert>
#include <math.h>
#include <iostream>
#include <iomanip>

#include <Rtypes.h>

#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
#include "../storage/event.h"

using std::cout;
using std::endl;

namespace Processors {

/* Takes an offset from the current _pos (0 is _pos), and returns the index
 * of the buffer element at that offset. Wraps around if necessary */
unsigned int LargeSynchronizer::offsetToIndex(int offset) const
{
  // Going backwards by 1 is the same as going forward by buffer size - 1
  if (offset < 0) offset = _bufferSize + offset;

  unsigned int pos = _numEvents % _bufferSize;
  offset = offset % _bufferSize;

  if (pos + offset >= _bufferSize)
    offset -= _bufferSize - pos;
  else
    offset += pos;

  return offset;
}

void LargeSynchronizer::addEvents(const Storage::Event* shiftEvent,
                                  const Storage::Event* staticEvent)
{
  // Get the index of the current write position
  const unsigned int index = offsetToIndex(0);

  const ULong64_t shiftClockDiff = (shiftEvent->getTimeStamp() - _lastShiftTime);
  const double shiftDiff = shiftClockDiff * _shiftDevice->getSyncRatio() /
      (double)_readOutWindow;
  _lastShiftTime = shiftEvent->getTimeStamp();

  _shiftBuffer[index] = shiftDiff;

  if (_numEvents < _bufferSize)
  {
    const ULong64_t staticClockDiff = (staticEvent->getTimeStamp() - _lastStaticTime);
    const double staticDiff = staticClockDiff * _staticDevice->getSyncRatio() /
        (double)_readOutWindow;
    _lastStaticTime = staticEvent->getTimeStamp();

    _staticBuffer[index] = staticDiff;
  }

  _numEvents++;
}

bool LargeSynchronizer::checkBuffer() const
{
  for (unsigned int i = 0; i < _bufferSize; i++)
  {
    const unsigned int index = offsetToIndex(i);
    const double staticVal = _staticBuffer[i];
    const double shiftVal = _shiftBuffer[index];
    if (fabs(staticVal - shiftVal) > _threshold) return false;
  }

  return true;
}

void LargeSynchronizer::printBuffers() const
{
  cout << std::setprecision(4);

  cout << "Static : [ ";
  for (unsigned int i = 0; i < _bufferSize; i++)
  {
    cout << std::setw(7) << _staticBuffer[i];
    if (i != _bufferSize - 1) cout << ",";
  }
  cout << " ]" << endl;

  cout << "Shift  : [ ";
  for (unsigned int i = 0; i < _bufferSize; i++)
  {
    const unsigned index = offsetToIndex(i);
    cout << std::setw(7) << _shiftBuffer[index];
    if (i != _bufferSize - 1) cout << ",";
  }
  cout << " ]" << endl;

  cout << std::setprecision(6);
}

LargeSynchronizer::LargeSynchronizer(const Mechanics::Device* shiftDevice,
                                     const Mechanics::Device* staticDevice,
                                     double threshold,
                                     unsigned int bufferSize,
                                     unsigned int readOutWindow,
                                     ULong64_t initialShiftTime,
                                     ULong64_t initialStaticTime) :
  _shiftDevice(shiftDevice),
  _staticDevice(staticDevice),
  _threshold(threshold),
  _bufferSize(bufferSize),
  _readOutWindow(readOutWindow),
  _lastShiftTime(initialShiftTime),
  _lastStaticTime(initialStaticTime),
  _shiftBuffer(0),
  _staticBuffer(0),
  _numEvents(0)
{
  _shiftBuffer = new double[_bufferSize];
  _staticBuffer = new double[_bufferSize];
  for (unsigned int i = 0; i < _bufferSize; i++)
  {
    _shiftBuffer[i] = 0;
    _staticBuffer[i] = 0;
  }
}

LargeSynchronizer::~LargeSynchronizer()
{
  delete[] _shiftBuffer;
  delete[] _staticBuffer;
}

}
