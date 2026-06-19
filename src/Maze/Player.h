#pragma once
#include "maze.h"

class Player {
public:
    Player(int id, Point3D start);

    // 尝试移动，若成功返回 true
    bool moveTo(int nx, int ny, int nz, const Maze& maze, const Player& other);
    // 记录轨迹（步数回退与更新）
    void recordStep(Maze& maze);
    // 备份位置（移动前调用）
    void backup();

    int getId() const noexcept;
    const Point3D& getPos() const noexcept;
    const Point3D& getPrev() const noexcept;
    int getStep() const noexcept;

private:
    int id;
    Point3D pos, prev;
    int step;
};