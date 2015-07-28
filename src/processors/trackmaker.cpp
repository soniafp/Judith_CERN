#include "trackmaker.h"

#include <cassert>
#include <vector>
#include <math.h>
#include <iostream>
#include <float.h>

#include "../storage/event.h"
#include "../storage/plane.h"
#include "../storage/track.h"
#include "../storage/cluster.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;

using namespace Storage;

namespace Processors {

void TrackMaker::searchPlane(Track* track, std::vector<Track*>& candidates,
                             unsigned int nplane)
{
  assert(nplane < _event->getNumPlanes() &&
         "TrackMaker: adding clusters in plane outside event range");
  assert(track && "TrackMaker: can't search plane with a void track");
  assert((int)nplane != _maskedPlane && "TrackMaker: ouch");

  const Plane* plane = _event->getPlane(nplane);

  // Search over clusters in this event
  bool matchedCluster = false;
  for (unsigned int ncluster = 0; ncluster < plane->getNumClusters() + 1; ncluster++)
  {
    Track* trialTrack = 0;

    assert(track->getNumClusters() && "TrackMaker: the track should have been seeded");
    Cluster* lastCluster = track->getCluster(track->getNumClusters() - 1);

    // Try to add the clusters to the track
    if (ncluster < plane->getNumClusters())
    {
      Cluster* cluster = plane->getCluster(ncluster);
      if (cluster->getTrack()) continue;

      const double errX = sqrt(pow(cluster->getPosErrX(), 2) + pow(lastCluster->getPosErrX(), 2));
      const double errY = sqrt(pow(cluster->getPosErrY(), 2) + pow(lastCluster->getPosErrY(), 2));

      // The real space distance between this cluster and the last
      const double distX = cluster->getPosX() - lastCluster->getPosX();
      const double distY = cluster->getPosY() - lastCluster->getPosY();
      const double distZ = cluster->getPosZ() - lastCluster->getPosZ();

      // Adjust the distance in X and Y to account for the slope, and normalize in sigmas
      const double sigDistX = (distX - _beamAngleX * distZ) / errX;
      const double sigDistY = (distY - _beamAngleY * distZ) / errY;

      const double dist = sqrt(pow(sigDistX, 2) + pow(sigDistY, 2));

      if (dist > _maxClusterDist) continue;

      // Found a good cluster, bifurcate the track and add the cluster
      matchedCluster = true;
      trialTrack = new Track(*track);
      trialTrack->addCluster(cluster);
    }
    // There were no more clusters in the track
    else if (!matchedCluster)
    {
      // No good clusters were found, use this track on the next plane
      trialTrack = track;
    }
    else
    {
      // At least one good cluster has been found
      delete track;
      continue; // This iteration of the loop isn't necessary
    }

    int planesRemaining = _event->getNumPlanes() - nplane - 1;
    // Adjust for the masked plane if applicable
    if (_maskedPlane >= 0 && _maskedPlane > (int)nplane)
      planesRemaining -= 1;

    assert(planesRemaining >= 0 && "TrackMaker: something went terribly wrong");

    const int requiredClusters = _minClusters - trialTrack->getNumClusters();

    // Check if it makes sense to continue building this track
    if (planesRemaining > 0 && requiredClusters <= planesRemaining)
    {
      const unsigned int nextPlane =
          (_maskedPlane == (int)nplane + 1) ? nplane + 2 : nplane + 1;
      // This call will delete the trial track when it is done with it
      searchPlane(trialTrack, candidates, nextPlane);
    }
    else if (trialTrack->getNumClusters() < _minClusters)
    {
      // This track can't continue and doesn't meet the cluster requirement
      delete trialTrack;
    }
    else
    {
      fitTrackToClusters(trialTrack);
      candidates.push_back(trialTrack);
    }
  }
}


void TrackMaker::generateTracks(Event* event,
                                double beamAngleX,
                                double beamAngleY,
                                int maskedPlane)
{
  if (event->getNumPlanes() < 3)
    throw "TrackMaker: can't generate tracks from event with less than 3 planes";
  if (event->getNumTracks() > 0)
    throw "TrackMaker: tracks already exist for this event";

  _beamAngleX = beamAngleX;
  _beamAngleY = beamAngleY;

  if (maskedPlane >= (int)event->getNumPlanes() && VERBOSE)
  {
    cout << "WARNING :: TrackMaker: masked plane outside range";
    maskedPlane = -1;
  }

  _event = event;
  _maskedPlane = maskedPlane;

  // This is the number of planes available for tracking (after masking)
  const unsigned int numPlanes =
      (_maskedPlane >= 0) ? _event->getNumPlanes() - 1 : _event->getNumPlanes();

  if (_minClusters > numPlanes)
    throw "TrackMaker: min clusters exceeds number of planes";

  const unsigned int maxSeedPlanes = numPlanes - _minClusters + 1;
  unsigned int numSeedPlanes = 0;
  if (_numSeedPlanes > maxSeedPlanes)
  {
    numSeedPlanes = maxSeedPlanes; // Requested number of seed planes exceeds max
    if (VERBOSE) cout << "WARNING :: TrackMaker: too many seed planes, adjusting" << endl;
  }
  else
  {
    numSeedPlanes = _numSeedPlanes; // Requested seed planes is OK
  }

  if (_maskedPlane >= 0 && _maskedPlane < (int)numSeedPlanes)
  {
    numSeedPlanes += 1; // Masking one of the seed planes, add one
  }

  if (numSeedPlanes < 1)
    throw "TrackMaker: can't make tracks with no seed planes";

  assert(numSeedPlanes < _event->getNumPlanes() &&
         "TrackMaker: num seed planes is outside the plane range");

  for (unsigned int nplane = 0; nplane < numSeedPlanes; nplane++)
  {
    if ((int)nplane == _maskedPlane) continue;

    const Plane* plane = _event->getPlane(nplane);

    // Each seed cluster generates a list of candidates from which the best is kept
    for (unsigned int ncluster = 0; ncluster < plane->getNumClusters(); ncluster++)
    {
      Cluster* cluster = plane->getCluster(ncluster);
      if (cluster->getTrack()) continue;

      std::vector<Track*> candidates;
      Track* seedTrack = new Track();
      seedTrack->setOrigin(cluster->getPosX(), cluster->getPosY());
      seedTrack->setOriginErr(cluster->getPosErrX(), cluster->getPosErrY());
      seedTrack->addCluster(cluster);

      const unsigned int nextPlane =
          (maskedPlane == (int)nplane + 1) ? nplane + 2 : nplane + 1;
      searchPlane(seedTrack, candidates, nextPlane);

      const unsigned int numCandidates = candidates.size();
      if (!numCandidates) continue;

      Track* bestCandidate = 0;

      // If there is only one candidate, use it
      if (numCandidates == 1)
      {
        bestCandidate = candidates.at(0);
      }
      // Otherwise find the best candidate for this seed
      else
      {
        // Find the longest candidate size
        unsigned int mostClusters = 0;
        std::vector<Track*>::iterator it;
        for (it = candidates.begin(); it != candidates.end(); ++it)
        {
          Track* candidate = *(it);
          if (candidate->getNumClusters() > mostClusters)
            mostClusters = candidate->getNumClusters();
        }

        // Find the best chi2 amongst tracks which match the most clusters
        for (it = candidates.begin(); it != candidates.end(); ++it)
        {
          Track* candidate = *(it);
          if (candidate->getNumClusters() < mostClusters) continue;
          if (!bestCandidate || candidate->getChi2() > bestCandidate->getChi2())
            bestCandidate = candidate;
        }

        // Delete the rest of the candidates
        for (it = candidates.begin(); it != candidates.end(); ++it)
        {
          Track* candidate = *(it);
          if (candidate != bestCandidate) delete candidate;
        }
      }

      assert(bestCandidate && "TrackMaker: failed to select a candidate");

      // Finalize the best candidate
      _event->addTrack(bestCandidate);
      for (unsigned int i = 0; i < bestCandidate->getNumClusters(); i++)
        bestCandidate->getCluster(i)->setTrack(bestCandidate);
    }
  }
}

int TrackMaker::linearFit(const unsigned int npoints, const double* independant,
                          const double* dependant, const double* uncertainty,
                          double& slope, double& slopeErr, double& intercept,
                          double& interceptErr, double& chi2, double& covariance)
{
  if (npoints < 3)
  {
    if (VERBOSE) cout << "WARN: can't fit a line to less than 3 points";
    return -1;
  }

  double* err = new double[npoints];

  slope = 0;
  slopeErr = 0;
  intercept = 0;
  interceptErr = 0;
  chi2 = 0;
  covariance = 0;

  // Check the uncertainties for problems
  for (unsigned int i = 0; i < npoints; i++)
  {
    assert(uncertainty[i] >= 0 &&
           "Processors: negative uncertainty for fit point");

    // Check if no uncertainty is specified (zeros are exact in binary)
    if (uncertainty[i] == 0.0) err[i] = 1.0;
    else err[i] = uncertainty[i];
  }

  // Regression variables
  double ss = 0;
  double sx = 0;
  double sy = 0;
  double sxoss = 0;
  double st2 = 0;

  for (unsigned int i = 0; i < npoints; i++) {
    const double wt = 1.0 / pow(uncertainty[i], 2);
    // Check for divison by 0
    if (!(wt <= DBL_MAX && wt >= -DBL_MAX))
    {
      if (VERBOSE) cout << "WARN: regresssion divided by 0 uncertainty" << endl;
      return -1;
    }
    ss += wt;
    sx += independant[i] * wt;
    sy += dependant[i] * wt;
  }

  sxoss = sx / ss;

  for (unsigned int i = 0; i < npoints; i++) {
    const double t = (independant[i] - sxoss) / uncertainty[i];
    st2 += t * t;
    slope += t * dependant[i] / uncertainty[i];
  }

  slope /= st2;
  intercept = (sy - sx * slope) / ss;

  // sy, sx, ss, st2 are good

  slopeErr = sqrt(1.0 / st2);
  interceptErr = sqrt((1.0 + sx * sx / (ss * st2)) / ss);

  for (unsigned int i = 0; i < npoints; i++)
    chi2 += pow((dependant[i] - intercept - slope * independant[i]) /
                uncertainty[i], 2);

  covariance = -sx / (ss * st2);

  delete[] err;

  return 0;
}

void TrackMaker::fitTrackToClusters(Track* track)
{
  const unsigned int npoints = track->getNumClusters();
  double* dependant = new double[npoints];
  double* independant = new double[npoints];
  double* uncertainty = new double[npoints];

  // Prepare variables for the regression output
  double originX = 0, originY = 0;
  double originErrX = 0, originErrY = 0;
  double slopeX = 0, slopeY = 0;
  double slopeErrX = 0, slopeErrY = 0;
  double chi2X = 0, chi2Y = 0;
  double covarianceX = 0, covarianceY = 0;

  for (unsigned int axis = 0; axis < 2; axis++)
  {
    // Fill the arrays with the points dependint on the loop (x or y axis)
    for (unsigned int npoint = 0; npoint < npoints; npoint++)
    {
      if (!axis)
      {
        dependant[npoint] = track->getCluster(npoint)->getPosX();
        uncertainty[npoint] = track->getCluster(npoint)->getPosErrX();
      }
      else
      {
        dependant[npoint] = track->getCluster(npoint)->getPosY();
        uncertainty[npoint] = track->getCluster(npoint)->getPosErrY();
      }
      independant[npoint] = track->getCluster(npoint)->getPosZ();
    }

    // Perform the regression
    if (!axis)
    {
      linearFit(npoints, independant, dependant, uncertainty,
                slopeX, slopeErrX, originX, originErrX, chi2X, covarianceX);
    }
    else
    {
      linearFit(npoints, independant, dependant, uncertainty,
                slopeY, slopeErrY, originY, originErrY, chi2Y, covarianceY);
    }
  }

  // Get a chi2 normalized to the number of DOF
  const double chi2 = (chi2X + chi2Y) / (2.0 * (double)(npoints - 2));

  track->setOrigin(originX, originY);
  track->setOriginErr(originErrX, originErrY);
  track->setSlope(slopeX, slopeY);
  track->setSlopeErr(slopeErrX, slopeErrY);
  track->setChi2(chi2);
  track->setCovariance(covarianceX, covarianceY);

  delete[] dependant;
  delete[] independant;
  delete[] uncertainty;
}

TrackMaker::TrackMaker(double maxClusterDist,
                       unsigned int numSeedPlanes,
                       unsigned int minClusters) :
  _maxClusterDist(maxClusterDist),
  _numSeedPlanes(numSeedPlanes),
  _minClusters(minClusters),
  _event(0),
  _maskedPlane(-1)
{
  if (minClusters < 3)
    throw "TrackMaker: min clusters needs to be at least 3";
  if (numSeedPlanes < 1)
    throw "TrackMaker: needs at least one seed plane";
}

}
