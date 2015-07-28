#ifndef TRACK_H
#define TRACK_H

#include <vector>

namespace Processors { class TrackMaker; }

namespace Storage {

class Cluster;

/*******************************************************************************
 * Track class contains all values which define one track, composed of some
 * number of clusters. It also provides a list of the clusters which comprise
 * it.
 */

class Track
{
private:
  double _originX;
  double _originY;
  double _originErrX;
  double _originErrY;
  double _slopeX;
  double _slopeY;
  double _slopeErrX;
  double _slopeErrY;
  double _covarianceX;
  double _covarianceY;
  double _chi2;

  unsigned int _numClusters;
  std::vector<Cluster*> _clusters;

  // NOTE: this isn't stored in file, its a place holder for doing DUT analysis
  unsigned int _numMatchedClusters;
  std::vector<Cluster*> _matchedClusters;

protected:
  int _index;

public:
  Track(const Track& old);
  Track();

  void addCluster(Cluster* cluster);
  void addMatchedCluster(Cluster* cluster);

  Cluster* getCluster(unsigned int n) const;
  Cluster* getMatchedCluster(unsigned int n) const;

  inline void setOrigin(double x, double y) { _originX = x; _originY = y; }
  inline void setOriginErr(double x, double y) { _originErrX = x; _originErrY = y; }
  inline void setSlope(double x, double y) { _slopeX = x; _slopeY = y; }
  inline void setSlopeErr(double x, double y) { _slopeErrX = x; _slopeErrY = y; }
  inline void setChi2(double chi2) { _chi2 = chi2; }
  inline void setCovariance(double x, double y) { _covarianceX = x; _covarianceY = y; }

  inline unsigned int getNumClusters() const { return _numClusters; }
  inline unsigned int getNumMatchedClusters() const { return _numMatchedClusters; }
  inline double getOriginX() const { return _originX; }
  inline double getOriginY() const { return _originY; }
  inline double getOriginErrX() const { return _originErrX; }
  inline double getOriginErrY() const { return _originErrY; }
  inline double getSlopeX() const { return _slopeX; }
  inline double getSlopeY() const { return _slopeY; }
  inline double getSlopeErrX() const { return _slopeErrX; }
  inline double getSlopeErrY() const { return _slopeErrY; }
  inline double getCovarianceX() const { return _covarianceX; }
  inline double getCovarianceY() const { return _covarianceY; }
  inline double getChi2() const { return _chi2; }
  inline int getIndex() const { return _index; }

  friend class Event;
  friend class Processors::TrackMaker;
};

}

#endif // TRACK_H
