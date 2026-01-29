#include <iostream>
#include "CustomClass.h"
#include "ObjectPool.h"
#include "ActorExample.h"

int main()
{
    // create pool with 2 pre-allocated objects
    ObjectPool<CustomClass> pool(2);

    CustomActor actor(pool);
    actor.start();

    // post messages to actor
    actor.post(CustomClass("hello", 1));
    actor.post(CustomClass("world", 2));

    // allow some time for actor to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    actor.stop();

    return 0;
}
