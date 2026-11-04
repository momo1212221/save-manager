#include <cassert>
#include "logic.hpp"
int main(){ assert(qol::wrap_slot(1,-1)==8); assert(qol::wrap_slot(8,1)==1); assert(qol::wrap_slot(4,1)==5); }
