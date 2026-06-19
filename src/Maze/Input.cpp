#include "input.h"
#include "Console.h"

std::optional<Action> Input::getPlayerAction(int playerId) {
    using namespace Console;
    if (playerId == 1) {
        if (isKeyDown(VK_UP))    return Action::MoveUp;
        if (isKeyDown(VK_DOWN))  return Action::MoveDown;
        if (isKeyDown(VK_LEFT))  return Action::MoveLeft;
        if (isKeyDown(VK_RIGHT)) return Action::MoveRight;
        if (isKeyDown('O'))      return Action::LayerUp;
        if (isKeyDown('P'))      return Action::LayerDown;
    }
    else {
        if (isKeyDown('W')) return Action::MoveUp;
        if (isKeyDown('S')) return Action::MoveDown;
        if (isKeyDown('A')) return Action::MoveLeft;
        if (isKeyDown('D')) return Action::MoveRight;
        if (isKeyDown('Q')) return Action::LayerUp;
        if (isKeyDown('E')) return Action::LayerDown;
    }
    return std::nullopt;
}

bool Input::isViewPrev() { return Console::isKeyDown('N'); }
bool Input::isViewNext() { return Console::isKeyDown('M'); }

bool Input::isAnyDirectionKey() {
    return Console::isKeyDown(VK_UP) || Console::isKeyDown(VK_DOWN) ||
        Console::isKeyDown(VK_LEFT) || Console::isKeyDown(VK_RIGHT) ||
        Console::isKeyDown('W') || Console::isKeyDown('S') ||
        Console::isKeyDown('A') || Console::isKeyDown('D');
}