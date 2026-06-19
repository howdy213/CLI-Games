#pragma once
#include <vector>
#include <string>

struct Point3D {
    int x, y, z;
    bool operator==(const Point3D&) const = default;
};

struct Cell {
    bool wall = false;
    int  step1 = 0;
    int  step2 = 0;
};

class Maze {
public:
    bool load(const std::string& filename);

    Cell& at(int x, int y, int z);
    const Cell& at(int x, int y, int z) const;

    int maxX() const noexcept;
    int maxY() const noexcept;
    int maxZ() const noexcept;

    // 清除步数大于指定值的轨迹
    void cleanTrails(int maxStep1, int maxStep2);

private:
    std::vector<std::vector<std::vector<Cell>>> cells;
    size_t sizeX = 0, sizeY = 0, sizeZ = 0;
};