#ifndef STORAGEIO_H
#define STORAGEIO_H

#include <string>
#include <vector>
#include <stack>
#include <set>

#include <Rtypes.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>

// NOTE: these sizes are used to initialize arrays of track, cluster and
// hit information. BUT these arrays are generated ONLY ONCE and re-used
// to load events. Vectors could have been used in the ROOT file format, but
// they would need to be constructed at each event reading step. 
#define MAX_TRACKS 1000
#define MAX_CLUSTERS 10000
#define MAX_HITS 10000

namespace Storage {

class Hit;
class Cluster;
class Plane;
class Track;
class Event;

/**
  * Interface for a `TFile` used for either input or output. Caches the event-
  * by-event objects generated on each read/write; i.e. hits, clusters, tracks
  * and the event object, but not planes.
  *
  * Local memory is filled either by an `Event` object, and then read into the
  * output `TFile`, or is filled by a `TFile` and then used to populate an
  * `Event` object. Given the amount of memory allocated to an `Event` object
  * (including all hits, clusters, tracks), all such objects are cached.
  *
  * Includes the ability to mask parts of the file (i.e. do not read/write
  * some variables, or entire sets of variables). Also provids the ability to
  * apply a noise mask either by flagging hits in noisy pixels as masked, or
  * by not reading/writing them entirely. Note that a passive (flagging) mask
  * will not store its outcome into the file.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class StorageIO {
private:
  // Disable copy and assignment operators
  StorageIO(const StorageIO&);
  StorageIO& operator=(const StorageIO&);

public:
  enum ContentFlags {
    NONE = 0,
    HITS = 1<<0,
    CLUSTERS = 1<<1,
    TRACKS = 1<<2,
    EVENTINFO = 1<<3,
  };

  enum FileMode {
    INPUT,
    OUTPUT
  };

  enum MaskMode {
    // Passive mode flags masked hits during read back but doesn't store this
    // information in a persitent way
    PASSIVE,
    // Remove mode removes flaged masks (they also won't be written)
    REMOVE
  };

  /** Struct allows user to pass a noise mask for each plane as a 1D vector
    * specifying how it is partitionned */
  struct NoiseMask {
    std::vector<bool> mask;
    size_t nrows;
    size_t ncols;
    NoiseMask() : nrows(0), ncols(0) {}
    inline bool at(size_t nrow, size_t ncol) { return mask[nrow*ncols+ncol]; }
  };

protected:
  /** File to read or write */
  TFile m_file;
  /** Remember if the file is being read or written */
  const FileMode m_fileMode;
  /** Number of planes in the device */
  size_t m_numPlanes;
  /** Strategy for dealing with masked hits */
  MaskMode m_maskMode;
  /** Number of events */
  Long64_t m_numEvents;
  /** Vector of NoiseMask objects for each plane. The objects are realtively
    * small and the vector won't be copied. */
  std::vector<NoiseMask> m_noiseMasks;

  // Cache objects so they aren't re-allocated at each iteration
  Event* m_event;  // Only one event exists ever in this object
  std::stack<Track*> m_cacheTracks;
  std::stack<Cluster*> m_cacheClusters;
  std::stack<Hit*> m_cacheHits;

  // NOTE: trees can easily be added and removed from a file. So each type
  // of information that might or might not be included in a file should be
  // in its own tree.

  // Trees containing event-by-event data for each plane
  std::vector<TTree*> m_hitsTrees;
  std::vector<TTree*> m_clustersTrees;
  // Trees global to the entire event
  TTree* m_tracksTree;
  TTree* m_eventInfoTree;

  // Keep track of disabled branches in each tree
  std::set<std::string> m_hitsBranchesOff;
  std::set<std::string> m_clustersBranchesOff;
  std::set<std::string> m_tracksBranchesOff;
  std::set<std::string> m_eventInfoBranchesOff;

  // Variables in which the storage is output on an event-by-event basis

  // NOTE: don't change the order of these declarations since they are used to
  // do one big memset to zero everything

  Int_t    numHits;
  Int_t    hitPixX[MAX_HITS];
  Int_t    hitPixY[MAX_HITS];
  Double_t hitPosX[MAX_HITS];
  Double_t hitPosY[MAX_HITS];
  Double_t hitPosZ[MAX_HITS];
  Int_t    hitValue[MAX_HITS];
  Int_t    hitTiming[MAX_HITS];
  Int_t    hitInCluster[MAX_HITS];

  Int_t    numClusters;
  Double_t clusterPixX[MAX_CLUSTERS];
  Double_t clusterPixY[MAX_CLUSTERS];
  Double_t clusterPixErrX[MAX_CLUSTERS];
  Double_t clusterPixErrY[MAX_CLUSTERS];
  Double_t clusterPosX[MAX_CLUSTERS];
  Double_t clusterPosY[MAX_CLUSTERS];
  Double_t clusterPosZ[MAX_CLUSTERS];
  Double_t clusterPosErrX[MAX_CLUSTERS];
  Double_t clusterPosErrY[MAX_CLUSTERS];
  Double_t clusterPosErrZ[MAX_CLUSTERS];
  Double_t clusterValue[MAX_CLUSTERS];
  Double_t clusterTiming[MAX_CLUSTERS];
  Int_t    clusterInTrack[MAX_CLUSTERS];

  ULong64_t timeStamp;
  ULong64_t frameNumber;
  Int_t     triggerOffset;
  Int_t     triggerInfo;
  Bool_t    invalid;

  Int_t    numTracks;
  Double_t trackSlopeX[MAX_TRACKS];
  Double_t trackSlopeY[MAX_TRACKS];
  Double_t trackSlopeErrX[MAX_TRACKS];
  Double_t trackSlopeErrY[MAX_TRACKS];
  Double_t trackOriginX[MAX_TRACKS];
  Double_t trackOriginY[MAX_TRACKS];
  Double_t trackOriginErrX[MAX_TRACKS];
  Double_t trackOriginErrY[MAX_TRACKS];
  Double_t trackCovarianceX[MAX_TRACKS];
  Double_t trackCovarianceY[MAX_TRACKS];
  Double_t trackChi2[MAX_TRACKS];

  /** Cached make new track only for friend Event class */
  Track& newTrack();
  /** Cached make new cluster only for friend Event class */
  Cluster& newCluster();
  /** Cached make new hit only for friend Event class */
  Hit& newHit();

public:
  StorageIO(
      const std::string& filePath,
      FileMode fileMode,
      size_t numPlanes=1);
  virtual ~StorageIO();

  /** Provides the `Event` object cleared to be filled. NOTE: this event is
    * overwritten whenever this method is called. */
  Event& newEvent();

  bool isHitsBranchOff(const std::string& name) const;
  bool isClustersBranchOff(const std::string& name) const;
  bool isTracksBranchOff(const std::string& name) const;
  bool isEventInfoBranchOff(const std::string& name) const;

  const std::set<std::string>& getHitsBranchesOff() const { return m_hitsBranchesOff; }
  const std::set<std::string>& getClustersBranchesOff() const { return m_clustersBranchesOff; }
  const std::set<std::string>& getTracksBranchesOff() const { return m_tracksBranchesOff; }
  const std::set<std::string>& getEventInfoBranchesOff() const { return m_eventInfoBranchesOff; }

  Long64_t getNumEvents() const { return m_numEvents; }
  size_t getNumPlanes() const { return m_numPlanes; }
  FileMode getFileMode() const { return m_fileMode; }
  MaskMode getMaskMode() const { return m_maskMode; }

  friend class Event;  // Access to cache
};

}

#endif // STORAGEIO_H
