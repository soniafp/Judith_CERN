#ifndef TRACKMAKER_H
#define TRACKMAKER_H

#include <vector>

namespace Storage { class Event; }
namespace Storage { class Cluster; }
namespace Storage { class Track; }

namespace Processors {

class TrackMaker
{
private:
  const double _maxClusterDist;
  const unsigned int _numSeedPlanes;
  const unsigned int _minClusters;
  double _beamAngleX;
  double _beamAngleY;

  Storage::Event* _event;
  int _maskedPlane;

  void searchPlane(Storage::Track* track, std::vector<Storage::Track*>& candidates,
                   unsigned int nplane);

public:
  TrackMaker(double maxClusterDist,
             unsigned int numSeedPlanes = 1,
             unsigned int minClusters = 3);

  void generateTracks(Storage::Event* event,
                      double beamAngleX = 0,
                      double beamAngleY = 0,
                      int maskedPlane = -1);

  static int linearFit(const unsigned int npoints, const double* independant,
                       const double* dependant, const double* uncertainty,
                       double& slope, double& slopeErr, double& intercept,
                       double& interceptErr, double& chi2, double& covariance);

  static void fitTrackToClusters(Storage::Track* track);
};

}

#endif // TRACKMAKER_H
