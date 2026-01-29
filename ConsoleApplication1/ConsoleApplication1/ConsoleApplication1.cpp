#include <iostream>
#include "CustomClass.h"
#include "ObjectPool.h"

int main()
{
    ObjectPool<CustomClass> pool(5);

    // acquire objects from pool
    CustomClass* a = pool.acquire("first", 1);
    CustomClass* b = pool.acquire("second", 2);

    std::cout << a->text << ": " << a->value << std::endl;
    std::cout << b->text << ": " << b->value << std::endl;

    // release back to pool
    pool.release(a);
    pool.release(b);

    return 0;
}
