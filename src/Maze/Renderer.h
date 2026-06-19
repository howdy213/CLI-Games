#pragma once
#include "maze.h"
#include "player.h"
#include <string>

class Renderer {
public:
    explicit Renderer(const Maze& maze);
    std::string buildFrame(int viewZ, const Player& p1, const Player& p2) const;

private:
    const Maze& maze;
};