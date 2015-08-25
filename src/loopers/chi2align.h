#ifndef CHI2ALIGN_H
#define CHI2ALIGN_H

#include <utility>
#include <list>
#include <vector>

#include <Math/IFunction.h>

#include "../mechanics/device.h"
#include "looper.h"

namespace Storage { class StorageIO; }
namespace Storage { class Cluster; }
namespace Processors { class ClusterMaker; }
namespace Processors { class TrackMaker; }

namespace Loopers {

class Chi2Align : public Looper
{
private:
  enum ParPos { ROTX, ROTY, ROTZ, TRANSX, TRANSY, NPARS };

  struct Tracklet {
    unsigned int n0;
    unsigned int n1;
    double x0;
    double y0;
    double z0;
    double ex0;
    double ey0;
    double x1;
    double y1;
    double z1;
    double ex1;
    double ey1;
    Tracklet() :
        n0(0),
        n1(0),
        x0(0),
        y0(0),
        z0(0),
        ex0(0),
        ey0(0),
        x1(0),
        y1(0),
        z1(0),
        ex1(0),
        ey1(0) {}
  };

  /** Deep within the belly of the beast, lies an interface to for a
    * multidimensional function with a method to compute gradients */
  class Chi2Minimizer : public ROOT::Math::IBaseFunctionMultiDim {
  private:
    Mechanics::Device& _device;
    // NOTE: vector of objects is appropriate here since all values should be
    // copied over, then the vector is no longer modified, and they must be
    // read sequentially.
    const std::vector<Tracklet> _tracklets;
    const unsigned int _ndim;

    double DoEval(const double* x) const;

  public:
    Chi2Minimizer(
      Mechanics::Device& device,
      std::list<Tracklet> tracklets) :
      _device(device),
      _tracklets(tracklets.begin(), tracklets.end()),
      _ndim((_device.getNumSensors()-1)*NPARS) {}

    Chi2Minimizer(
      Mechanics::Device& device,
      std::vector<Tracklet> tracklets) :
      _device(device),
      _tracklets(tracklets),
      _ndim((_device.getNumSensors()-1)*NPARS) {}

    ROOT::Math::IBaseFunctionMultiDim* Clone() const;
    inline unsigned int NDim() const { return _ndim; }
  };

  Mechanics::Device* _refDevice;
  Processors::ClusterMaker* _clusterMaker;

  double _maxChi2;

public:
  Chi2Align(/* Use if you need mechanics (noise mask, pixel arrangement ...) */
            Mechanics::Device* refDevice,
            /* Use if the looper needs to make clusters and/or tracks... */
            Processors::ClusterMaker* clusterMaker,
            /* These arguments are needed to be passed to the base looper class */
            Storage::StorageIO* refInput,
            ULong64_t startEvent = 0,
            ULong64_t numEvents = 0,
            Long64_t eventSkip = 1);

  void loop();

  void setMaxChi2(double value);
};

}

#endif
