#pragma once
#include <array>
#include <cstddef>

namespace itemtrade_logic {

enum class DecisionKind { None, SellMain, TradeChild };
struct Decision { DecisionKind kind = DecisionKind::None; int childSlot = 0; };

inline Decision Decide(int mainFreeSlots, int mainSellThreshold,
                       const std::array<int, 6>& childFreeSlots,
                       const std::array<bool, 6>& childReady) {
    if (mainFreeSlots <= mainSellThreshold) return {DecisionKind::SellMain, 0};
    for (int i = 0; i < 6; ++i) {
        if (childReady[static_cast<std::size_t>(i)] && childFreeSlots[static_cast<std::size_t>(i)] <= 0)
            return {DecisionKind::TradeChild, i + 1};
    }
    return {};
}

} // namespace itemtrade_logic
