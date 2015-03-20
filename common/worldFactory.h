#ifndef WorldFactory_h
#define WorldFactory_h

#include "noise/PerlinNoise.h"
#include "voxel/block.h"
#include "vector3.h"

// thread unsafe
class WorldFactory
{
public:
  WorldFactory(int, int, int);
  int getMinElevation() const;
  int getMaxElevation() const;
  int elevation(Vector3 v);
  Blocktype computeBlockType(Vector3 globalXYZ);

private:
  int minElevation, maxElevation;
  PerlinNoise noise;
  int noiseDepth;
};
#endif
