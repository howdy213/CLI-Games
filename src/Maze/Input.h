#pragma once
#include <optional>

enum class Action { MoveUp, MoveDown, MoveLeft, MoveRight, LayerUp, LayerDown };

class Input {
public:
    static std::optional<Action> getPlayerAction(int playerId);
    static bool isViewPrev();
    static bool isViewNext();
    static bool isAnyDirectionKey();
};