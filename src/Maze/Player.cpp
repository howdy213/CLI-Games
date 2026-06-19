#include "player.h"

Player::Player(int id, Point3D start)
    : id(id), pos(start), prev(start), step(1) {
}

bool Player::moveTo(int nx, int ny, int nz, const Maze& maze, const Player& other) {
    if (maze.at(nx, ny, nz).wall) return false;
    if (other.pos.x == nx && other.pos.y == ny && other.pos.z == nz) return false;
    prev = pos;
    pos = { nx, ny, nz };
    return true;
}

void Player::recordStep(Maze& maze) {
    Cell& cur = maze.at(pos.x, pos.y, pos.z);
    int& curStep = (id == 1) ? cur.step1 : cur.step2;
    Cell& old = maze.at(prev.x, prev.y, prev.z);
    int& oldStep = (id == 1) ? old.step1 : old.step2;

    if (curStep < step && curStep != 0) {
        step = curStep;          // 回到之前访问时的步数
    }
    else {
        ++step;
        curStep = step;
    }

    // 如果当前格步数刚好比旧格小 1，清除旧格的轨迹（表示回溯）
    if (curStep == oldStep - 1) {
        oldStep = 0;
    }
}

void Player::backup() { prev = pos; }

int Player::getId() const noexcept { return id; }
const Point3D& Player::getPos() const noexcept { return pos; }
const Point3D& Player::getPrev() const noexcept { return prev; }
int Player::getStep() const noexcept { return step; }