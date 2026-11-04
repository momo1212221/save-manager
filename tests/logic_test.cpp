#include <cassert>
#include "logic.hpp"
int main(){
    assert(qol::wrap_index(0,5,-1)==4);
    assert(qol::wrap_index(4,5,1)==0);
    assert(qol::wrap_index(2,5,1)==3);
    assert(qol::wrap_index(3,0,1)==0);
}
