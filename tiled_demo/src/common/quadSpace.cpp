#include "common/quadSpace.h"

void *QuadSpace::allocBuffer(size_t sizeofAnObject)
{
    return malloc(sizeofAnObject * NumSubSpaces);
}
