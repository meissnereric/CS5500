#include "worldFactory.h"

WorldFactory::WorldFactory(int minElevation, int maxElevation, int noiseDepth)
{
  this.minElevation = minElevation;
  this.maxElevation = maxElevation;
  noise = PerlinNoise();
  this.noiseDepth = noiseDepth;
}

int WorldFactory::getMinElevation() const
{
  return minElevation;
}

int WorldFactory::getMaxElevation() const
{
  return maxElevation;
}

int WorldFactory::elevation(Vector3 v)
{
  double weight = noise.turbulence2D(v.x, v.y, noiseDepth);
  return minElevation * (1 - weight) + maxElevation * weight;
}

Blocktype computeBlockType(Vector3 globalXYZ){
   if(globalXYZ.z <= elevation(globalXYZ))
       return Blocktype::Inactive;
   else //Compute Biome/etc here
       return Blocktype::Stone;
}
