#ifndef TRACK_H
#define TRACK_H

#include <vector>

namespace Storage {

class Cluster;

/**
  * Collection of clusters on different planes which are considered to have 
  * been created by the same particle. Also stores the parameters describing
  * a straight line passing through those clusters.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Track {
protected:
  /** List of clusters comprising the track */
  std::vector<Cluster*> m_clusters;
  /** List of clusters matched to this track but not part of the track */
  std::vector<Cluster*> m_matchedClusters;
  /** Origin of track's linear fit in global space x coordinate */
  double m_originX;
  double m_originY;
  /** Standard deviation of track's linear fit in global space x coordinate */
  double m_originErrX;
  double m_originErrY;
  /** Slope of track linear fit along global x as a function of global z */
  double m_slopeX;
  double m_slopeY;
  /** Standard deviation of track's linear fit x slope */
  double m_slopeErrX;
  double m_slopeErrY;
  /** Covariance of track's linear fit origin and slope in global x */
  double m_covarianceX;
  double m_covarianceY;
  /** Chi^2 of track's linear fit normalized to D.O.F in both X and Y */
  double m_chi2;
  /** Index of this track in the list of all tracks. Needed to build cluster-
    * track associations when reading from disk. */
  int m_index;

  /** Clear values so the object can be re-used */
  void clear();

public:
  Track();
  ~Track() {}

  /** Add a cluster to the track. */
  void addCluster(Cluster& cluster);
  /** Set all cluster in the track. */
  void setClusters(const std::vector<Cluster*>& clusters);
  /** Match a cluster to this track */
  void addMatchedCluster(Cluster& cluster);
  /** Set all clusters matched to this track */
  void setMatchedClusters(const std::vector<Cluster*>& clusters);

  Cluster& getCluster(size_t n) const;
  Cluster& getMatchedCluster(size_t n) const;
  inline size_t getNumClusters() const { return m_clusters.size(); }
  inline size_t getNumMatchedClusters() const { return m_matchedClusters.size(); }
  inline double getOriginX() const { return m_originX; }
  inline double getOriginY() const { return m_originY; }
  inline double getOriginErrX() const { return m_originErrX; }
  inline double getOriginErrY() const { return m_originErrY; }
  inline double getSlopeX() const { return m_slopeX; }
  inline double getSlopeY() const { return m_slopeY; }
  inline double getSlopeErrX() const { return m_slopeErrX; }
  inline double getSlopeErrY() const { return m_slopeErrY; }
  inline double getCovarianceX() const { return m_covarianceX; }
  inline double getCovarianceY() const { return m_covarianceY; }
  inline double getChi2() const { return m_chi2; }

  inline int getIndex() const { return m_index; }

  inline void setOrigin(double x, double y) { m_originX = x; m_originY = y; }
  inline void setOriginErr(double x, double y) { m_originErrX = x; m_originErrY = y; }
  inline void setSlope(double x, double y) { m_slopeX = x; m_slopeY = y; }
  inline void setSlopeErr(double x, double y) { m_slopeErrX = x; m_slopeErrY = y; }
  inline void setCovariance(double x, double y) { m_covarianceX = x; m_covarianceY = y; }
  inline void setChi2(double chi2) { m_chi2 = chi2; }

  friend class StorageIO;  // Sets values
  friend class Event;  // Access index
};

}

#endif // TRACK_H
