#include "trade_logic.h"
#include <cassert>
#include <array>
using namespace itemtrade_logic;
int main() {
    const std::array<bool,6> ready{true,true,true,true,true,true};
    {
        std::array<int,6> c{0,0,5,8,9,12};
        auto d=Decide(6,6,c,ready); assert(d.kind==DecisionKind::SellMain && d.childSlot==0);
    }
    {
        std::array<int,6> c{4,0,3,2,1,1};
        auto d=Decide(7,6,c,ready); assert(d.kind==DecisionKind::TradeChild && d.childSlot==2);
    }
    {
        std::array<int,6> c{0,0,0,0,0,0};
        auto d=Decide(30,6,c,ready); assert(d.kind==DecisionKind::TradeChild && d.childSlot==1);
    }
    {
        std::array<int,6> c{1,2,3,4,5,6};
        auto d=Decide(30,6,c,ready); assert(d.kind==DecisionKind::None);
    }
}
