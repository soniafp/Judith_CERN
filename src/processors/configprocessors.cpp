#include "configprocessors.h"

#include <cassert>

#include "../mechanics/device.h"
#include "../configparser.h"
#include "trackmaker.h"
#include "trackmatcher.h"
#include "clustermaker.h"

namespace Processors {

TrackMaker* generateTrackMaker(const ConfigParser& config, bool align)
{
  double maxClusterSep = -1;
  unsigned int numSeedPlanes = 1;
  unsigned int minClusters = 3;

  const char* header = align ? "Tracking Align" : "Tracking";
  const char* footer = align ? "End Tracking Align" : "End Tracking";

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare(footer))
    {
      TrackMaker* tracker = new TrackMaker(maxClusterSep, numSeedPlanes, minClusters);
      return tracker;
    }

    if (row->isHeader)
      continue;

    if (row->header.compare(header))
      continue; // Skip non-device rows

    if (!row->key.compare("seed planes"))
      numSeedPlanes = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("min hit planes"))
      minClusters = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("max cluster dist"))
      maxClusterSep = ConfigParser::valueToNumerical(row->value);
    else
      throw "Processors: can't parse track maker row";
  }

  throw "Processors: didn't produce a track maker";
}

ClusterMaker* generateClusterMaker(const ConfigParser& config)
{
  unsigned int maxSeparationX = 0;
  unsigned int maxSeparationY = 0;
  double maxSeparation = 0;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Clustering"))
    {
      if (maxSeparationX == 0 && maxSeparationY == 0 && maxSeparation == 0)
        throw "Processors: not enough parameters to produce cluster maker";

      ClusterMaker* clusterMaker =
          new ClusterMaker(maxSeparationX, maxSeparationY, maxSeparation);

      return clusterMaker;
    }

    if (row->isHeader)
      continue;

    if (row->header.compare("Clustering"))
      continue; // Skip non-device rows

    if (!row->key.compare("separation x"))
      maxSeparationX = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("separation y"))
      maxSeparationY = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("separation"))
      maxSeparation = ConfigParser::valueToNumerical(row->value);
    else
      throw "Processors: can't parse cluster maker row";
  }

  throw "Processors: didn't produce a cluster maker";
}

}
