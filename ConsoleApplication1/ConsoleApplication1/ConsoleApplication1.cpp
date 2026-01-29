#include <iostream>
#include "CustomClass.h"
#include "ObjectPool.h"

int main()
{
    // create pool with 2 pre-allocated objects
    ObjectPool<CustomClass> pool(2);
    std::cout << "capacity: " << pool.capacity() << ", available: " << pool.available() << std::endl;

    // acquire two objects
    CustomClass* c1 = pool.acquire("obj1", 10);
    CustomClass* c2 = pool.acquire("obj2", 20);
    std::cout << c1->text << ": " << c1->value << std::endl;
    std::cout << c2->text << ": " << c2->value << std::endl;
    std::cout << "available after acquire: " << pool.available() << std::endl;

    // release one object back to pool
    pool.release(c1);
    std::cout << "available after release c1: " << pool.available() << std::endl;

    // acquire another object (should reuse released one)
    CustomClass* c3 = pool.acquire("obj3", 30);
    std::cout << c3->text << ": " << c3->value << std::endl;
    std::cout << "available after acquiring c3: " << pool.available() << std::endl;

    // return objects to pool
    pool.release(c2);
    pool.release(c3);
    std::cout << "final available: " << pool.available() << std::endl;

    return 0;
}
