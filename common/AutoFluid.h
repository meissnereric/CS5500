#ifndef _AUTO_FLUID_
#define _AUTO_FLUID_

#include <map>
#include <tuple>
#include <tbb/reader_writer_lock.h>
#include <tbb/atomic.h>
#include <vector>
//#include "voxel/chunkmanager.h"


typedef std::tuple<int,int,int> Point3D;
typedef tbb::reader_writer_lock RWLock;

typedef std::map<int,tbb::atomic<int>*> FluidZMap;
typedef std::pair<RWLock*,FluidZMap*> FluidZMapWithLock;

typedef std::map<int,FluidZMapWithLock*> FluidYMap;
typedef std::pair<RWLock*,FluidYMap*> FluidYMapWithLock;

typedef std::map<int, FluidYMapWithLock*> FluidXMap;
typedef std::pair<RWLock*,FluidXMap*> FluidXMapWithLock;

class FluidEngine {
public :
    const int FULL = 32;
    
    FluidEngine();
    ~FluidEngine();
    
    void setChunkManager(ChunkManager CM){this->cm = CM;}
    int getValue(Point3D);
    
    void fill(Point3D);
    
    void increment(Point3D);
    void decrement(Point3D);
    
    void distribute(Point3D);
    void distribute(std::vector<Point3D>);
    void updateAll(Point3D, Point3D);
    
private :
    tbb::atomic<int>* getOrCreate(Point3D point);
    FluidXMapWithLock* xMapWithLock;
    ChunkManager cm;
    bool canMove(Point3D fromPos, Point3D toPos);
    bool isBlocked(Point3D toPos);

};
// BlockType ChunkManager::get(int x, int y, int z);

#endif
