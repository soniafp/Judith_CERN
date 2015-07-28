#include "hit.h"

#include <cassert>
#include <iostream>

#include "cluster.h"
#include "plane.h"

using std::cout;
using std::endl;

namespace Storage {

void Hit::print()
{
  cout << "\nHIT:\n"
       << "  Pix: (" << getPixX() << " , " << getPixY() << ")\n"
       << "  Pos: (" << getPosX() << " , " << getPosY() << " , " << getPosZ() << ")\n"
       << "  Value: " << getValue() << "\n"
       << "  Timing: " << getTiming() << "\n"
       << "  Cluster: " << getCluster() << "\n"
       << "  Plane: "  << getPlane() << endl;
}

void Hit::setCluster(Cluster *cluster)
{
  assert(!_cluster && "Hit: can't cluster an already clustered hit.");
  _cluster = cluster;
}

Cluster* Hit::getCluster() const { return _cluster; }

Plane* Hit::getPlane() const { return _plane; }

Hit::Hit() :
  _pixX(0), _pixY(0), _posX(0), _posY(0), _posZ(0),
  _value(0), _timing(0), _cluster(0), _plane(0)
{ }

}
