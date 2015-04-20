#include "AutoFluid.h"
#include <vector>
#include <iostream>
#include <tbb/atomic.h>
#include<tbb/parallel_for.h>

FluidEngine::FluidEngine(){
    RWLock* rwLock = new RWLock();
    FluidXMap* xMap = new FluidXMap();
    
    xMapWithLock = new FluidXMapWithLock(rwLock, xMap);
}

///////////////////////////////////////////////////////////////////////////////

void destroy(FluidZMap::iterator zIt, FluidZMap* zMap){
    tbb::atomic<int>* atomicValue = zIt->second;
    zMap->erase(zIt);
    delete atomicValue;
}

void destroy(FluidYMap::iterator yIt, FluidYMap* yMap){
    FluidZMapWithLock* zMapWithLock = yIt->second;
    RWLock* zLock = zMapWithLock->first;
    FluidZMap* zMap = zMapWithLock->second;

    for(FluidZMap::iterator zIt=zMap->begin(); zIt!=zMap->end(); zIt++){
        destroy(zIt, zMap);
    }

    yMap->erase(yIt);
    
    delete zMap;
    delete zLock;
    delete zMapWithLock;
}

void destroy(FluidXMap::iterator xIt, FluidXMap* xMap){
    FluidYMapWithLock* yMapWithLock = xIt->second;
    RWLock* yLock = yMapWithLock->first;
    FluidYMap* yMap = yMapWithLock->second;

    for(FluidYMap::iterator yIt=yMap->begin(); yIt!=yMap->end(); yIt++){
        destroy(yIt, yMap);
    }
    
    xMap->erase(xIt);
    
    delete yMap;
    delete yLock;
    delete yMapWithLock;
}

FluidEngine::~FluidEngine(){
    RWLock* xLock = xMapWithLock->first;
    FluidXMap* xMap = xMapWithLock->second;
    
    for(FluidXMap::iterator xIt=xMap->begin(); xIt!=xMap->end(); xIt++){
        destroy(xIt,xMap);
    }
    
    delete xMap;
    delete xLock;
    delete xMapWithLock;
}

////////////////////////////////////////////////////////////////////////////

int FluidEngine::getValue(Point3D point){
    int x = std::get<0>(point);
    int y = std::get<1>(point);
    int z = std::get<2>(point);
    
    RWLock::scoped_lock_read xReadLock(*xMapWithLock->first);
    FluidXMap::iterator xIt = xMapWithLock->second->find(x);
    if(xIt != xMapWithLock->second->end()){
        FluidYMapWithLock* yMapWithLock = xIt->second;
        RWLock::scoped_lock_read yReadLock(*yMapWithLock->first);
        FluidYMap::iterator yIt = yMapWithLock->second->find(y);
        if(yIt != yMapWithLock->second->end()){
            FluidZMapWithLock* zMapWithLock = yIt->second;
            RWLock::scoped_lock_read zReadLock(*zMapWithLock->first);
            FluidZMap::iterator zIt = zMapWithLock->second->find(z);
            if(zIt != zMapWithLock->second->end()){
                tbb::atomic<int>* valueAtomic = zIt->second;
                return *valueAtomic;
            }
        }
    }
    //else
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
    
void init(FluidZMapWithLock* zMapWithLock, int z){
    zMapWithLock->first->lock();
    if(zMapWithLock->second->find(z) == zMapWithLock->second->end()){
        tbb::atomic<int>* valueAtomic = new tbb::atomic<int>(0);
        zMapWithLock->second->insert(std::make_pair(z, valueAtomic));
    }
    zMapWithLock->first->unlock();
}

void init(FluidYMapWithLock* yMapWithLock, int y){
    yMapWithLock->first->lock();
    if(yMapWithLock->second->find(y) == yMapWithLock->second->end()){
        RWLock* zLock = new RWLock();
        FluidZMap* zMap = new FluidZMap();
        FluidZMapWithLock* zMapWithLock = new FluidZMapWithLock(zLock, zMap);
        yMapWithLock->second->insert(std::make_pair(y, zMapWithLock));
    }
    yMapWithLock->first->unlock();
}

void init(FluidXMapWithLock* xMapWithLock, int x){
    xMapWithLock->first->lock();
    if(xMapWithLock->second->find(x) == xMapWithLock->second->end()){
        RWLock* yLock = new RWLock();
        FluidYMap* yMap = new FluidYMap();
        FluidYMapWithLock* yMapWithLock = new FluidYMapWithLock(yLock, yMap);
        xMapWithLock->second->insert(std::make_pair(x, yMapWithLock));
    }
    xMapWithLock->first->unlock();
}

tbb::atomic<int>* FluidEngine::getOrCreate(Point3D point){
    int x = std::get<0>(point);
    int y = std::get<1>(point);
    int z = std::get<2>(point);
    
    tbb::atomic<int>* valueAtomic;
    
    xMapWithLock->first->lock_read();
    FluidXMap::iterator xIt;
    while((xIt=xMapWithLock->second->find(x)) == xMapWithLock->second->end()){
        xMapWithLock->first->unlock();
        init(xMapWithLock, x);
        xMapWithLock->first->lock_read();
    }{
        FluidYMapWithLock* yMapWithLock = xIt->second;
        yMapWithLock->first->lock_read();
        FluidYMap::iterator yIt;
        while((yIt=yMapWithLock->second->find(y)) == yMapWithLock->second->end()){
            yMapWithLock->first->unlock();
            init(yMapWithLock, y);
            yMapWithLock->first->lock_read();
        }{
            FluidZMapWithLock* zMapWithLock = yIt->second;
            zMapWithLock->first->lock_read();
            FluidZMap::iterator zIt;
            while((zIt=zMapWithLock->second->find(z)) == zMapWithLock->second->end()){
                zMapWithLock->first->unlock();
                init(zMapWithLock, z);
                zMapWithLock->first->lock_read();
            }{
                valueAtomic = zIt->second;
            }
            zMapWithLock->first->unlock();
        }
        yMapWithLock->first->unlock();
    }
    xMapWithLock->first->unlock();
    
    return valueAtomic;
}

void FluidEngine::fill(Point3D point){
    tbb::atomic<int>* valueAtomic = getOrCreate(point);
    (*valueAtomic) = FULL;
}
    
void FluidEngine::increment(Point3D point){
    tbb::atomic<int>* valueAtomic = getOrCreate(point);
    (*valueAtomic)++;
}
void FluidEngine::decrement(Point3D point){
    int x = std::get<0>(point);
    int y = std::get<1>(point);
    int z = std::get<2>(point);
    
    xMapWithLock->first->lock_read();
    FluidXMap::iterator xIt = xMapWithLock->second->find(x);
    if(xIt != xMapWithLock->second->end()){
        
        FluidYMapWithLock* yMapWithLock = xIt->second;
        yMapWithLock->first->lock_read();
        FluidYMap::iterator yIt = yMapWithLock->second->find(y);
        if(yIt != yMapWithLock->second->end()){
            
            FluidZMapWithLock* zMapWithLock = yIt->second;
            zMapWithLock->first->lock_read();
            FluidZMap::iterator zIt = zMapWithLock->second->find(z);
            if(zIt != zMapWithLock->second->end()){
                tbb::atomic<int>* valueAtomic = zIt->second;
                int remaining = valueAtomic->fetch_and_decrement();
                
                if(remaining<=0){
                    zMapWithLock->first->unlock();
                    zMapWithLock->first->lock();
                    zIt=zMapWithLock->second->find(z);
                    if(zIt!=zMapWithLock->second->end() && zIt->second<=0){
                        destroy(zIt,zMapWithLock->second);
                    }
                }
            }
            
            bool zMapEmpty = zMapWithLock->second->empty();
            zMapWithLock->first->unlock();
            if(zMapEmpty){
                yMapWithLock->first->unlock();
                yMapWithLock->first->lock();
                yIt = yMapWithLock->second->find(y);
                if(yIt!=yMapWithLock->second->end() && yIt->second->second->empty()){
                    destroy(yIt, yMapWithLock->second);
                }
            }
        }
        
        bool yMapEmpty = yMapWithLock->second->empty();
        yMapWithLock->first->unlock();
        if(yMapEmpty){
            xMapWithLock->first->unlock();
            xMapWithLock->first->lock();
            xIt = xMapWithLock->second->find(x);
            if(xIt!=xMapWithLock->second->end() && xIt->second->second->empty()){
                destroy(xIt, xMapWithLock->second);
            }
        }
    }
    xMapWithLock->first->unlock();
}

//////////////////////////////////////////////////////////////////////////////
    
void FluidEngine::distribute(Point3D point){
    int x = std::get<0>(point);
    int y = std::get<1>(point);
    int z = std::get<2>(point);
    
    auto down = Point3D(x,y,z-1);
    if (canMove(point, down)){
        increment(down);
        decrement(point);
    }
    auto left = Point3D(x, y+1,z);
    if(canMove(point, left)){
        increment(left);
        decrement(point);
    }
    auto right = Point3D(x, y-1,z);
    if(canMove(point, right)){
        increment(right);
        decrement(point);
    }
    auto forward = Point3D(x+1, y,z);
    if(canMove(point, forward)){
        increment(forward);
        decrement(point);
    }
    auto backward = Point3D(x-1, y,z);
    if(canMove(point, backward)){
        increment(backward);
        decrement(point);
    }
    
}

bool FluidEngine::canMove(Point3D fromPos, Point3D toPos){
    auto toWater = getValue(toPos);
    auto fromWater = getValue(fromPos);
    return (!isBlocked(toPos) && !(toWater >= fromWater) );
}

bool FluidEngine::isBlocked(Point3D toPos){
    auto block = cm.get(std::get<0>(toPos), std::get<1>(toPos), std::get<2>(toPos));
    return !(block == NULL || block == 0);
}

void FluidEngine::distribute(std::vector<Point3D> points){
    tbb::parallel_for(size_t(0), size_t(points.size()), [&] (size_t index) { distribute(points[index]); });
}
void FluidEngine::updateAll(Point3D lowerBound, Point3D upperBound){
    int xLower = std::get<0>(lowerBound);
    int yLower = std::get<1>(lowerBound);
    int zLower = std::get<2>(lowerBound);
    
    int xUpper = std::get<0>(upperBound);
    int yUpper = std::get<1>(upperBound);
    int zUpper = std::get<2>(upperBound);
    
    std::vector<Point3D> points;
    
    {
        RWLock::scoped_lock_read xReadLock(*xMapWithLock->first);
        FluidXMap::iterator xLowerIt = xMapWithLock->second->lower_bound(xLower);
        FluidXMap::iterator xUpperIt = xMapWithLock->second->upper_bound(xUpper);
        for(FluidXMap::iterator xIt=xLowerIt; xIt!=xUpperIt; xIt++){
            FluidYMapWithLock* yMapWithLock = xIt->second;
            RWLock::scoped_lock_read yReadLock(*yMapWithLock->first);
            FluidYMap::iterator yLowerIt = yMapWithLock->second->lower_bound(yLower);
            FluidYMap::iterator yUpperIt = yMapWithLock->second->upper_bound(yUpper);
            for(FluidYMap::iterator yIt=yLowerIt; yIt!=yUpperIt; yIt++){
                FluidZMapWithLock* zMapWithLock = yIt->second;
                RWLock::scoped_lock_read zReadLock(*zMapWithLock->first);
                FluidZMap::iterator zLowerIt = zMapWithLock->second->lower_bound(zLower);
                FluidZMap::iterator zUpperIt = zMapWithLock->second->upper_bound(zUpper);
                for(FluidZMap::iterator zIt=zLowerIt; zIt!=zUpperIt; zIt++){
                    points.push_back(Point3D(xIt->first, yIt->first, zIt->first));
                }
            }
        }
    }
    
    distribute(points);
}
