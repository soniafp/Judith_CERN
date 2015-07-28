#ifndef CONFIGPROCESSORS_H
#define CONFIGPROCESSORS_H

class ConfigParser;

namespace Processors {

class TrackMaker;
class TrackMatcher;
class ClusterMaker;

TrackMaker* generateTrackMaker(const ConfigParser& config, bool align = false);

ClusterMaker* generateClusterMaker(const ConfigParser& config);

}

#endif // CONFIGURE_H
