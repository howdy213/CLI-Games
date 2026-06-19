#pragma once
#include "Maze.h"
#include "Player.h"
#include "Renderer.h"
#include "Input.h"

class Game {
public:
    Game();
    bool init();
    void run();

private:
    bool tryMove(Player& p, Action action);
    bool checkEndCondition() const;

    Maze maze;
    Player p1, p2;
    int viewZ;
    Renderer renderer;
};