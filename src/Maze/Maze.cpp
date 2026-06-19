#include "Maze.h"
#include <fstream>
#include <stdexcept>

bool Maze::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return false;

    std::vector<std::vector<std::vector<Cell>>> layers;
    std::vector<std::vector<Cell>> currentLayer;
    std::vector<Cell> currentRow;

    int token;
    bool finished = false;
    while (!finished && file >> token) {
        switch (token) {
        case 0: currentRow.push_back({ false, 0, 0 }); break;
        case 1: currentRow.push_back({ true,  0, 0 }); break;
        case 2: // 换行
            if (!currentRow.empty()) {
                currentLayer.push_back(std::move(currentRow));
                currentRow.clear();
            }
            break;
        case 3: // 换层
            if (!currentRow.empty()) {
                currentLayer.push_back(std::move(currentRow));
                currentRow.clear();
            }
            if (!currentLayer.empty()) {
                layers.push_back(std::move(currentLayer));
                currentLayer.clear();
            }
            break;
        case 4: // 结束
            if (!currentRow.empty()) {
                currentLayer.push_back(std::move(currentRow));
                currentRow.clear();
            }
            if (!currentLayer.empty()) {
                layers.push_back(std::move(currentLayer));
                currentLayer.clear();
            }
            finished = true;
            break;
        default: break;
        }
    }

    if (layers.empty()) return false;

    sizeZ = layers.size();
    sizeY = sizeX = 0;
    for (auto& layer : layers) {
        if (layer.size() > sizeY) sizeY = layer.size();
        for (auto& row : layer)
            if (row.size() > sizeX) sizeX = row.size();
    }

    const int dimX = static_cast<int>(sizeX) + 2;
    const int dimY = static_cast<int>(sizeY) + 2;
    const int dimZ = static_cast<int>(sizeZ) + 2;
    cells.assign(dimZ, std::vector(dimY, std::vector<Cell>(dimX)));

    for (int z = 0; z < dimZ; ++z)
        for (int y = 0; y < dimY; ++y)
            for (int x = 0; x < dimX; ++x) {
                if (x == 0 || x == dimX - 1 || y == 0 || y == dimY - 1 || z == 0 || z == dimZ - 1)
                    cells[z][y][x].wall = true;
            }

    // 填入文件数据
    for (size_t z = 0; z < sizeZ; ++z)
        for (size_t y = 0; y < layers[z].size(); ++y)
            for (size_t x = 0; x < layers[z][y].size(); ++x)
                cells[z + 1][y + 1][x + 1] = layers[z][y][x];

    return true;
}

Cell& Maze::at(int x, int y, int z) {
    return cells[z][y][x];
}

const Cell& Maze::at(int x, int y, int z) const {
    return cells[z][y][x];
}

int Maze::maxX() const noexcept { return static_cast<int>(sizeX); }
int Maze::maxY() const noexcept { return static_cast<int>(sizeY); }
int Maze::maxZ() const noexcept { return static_cast<int>(sizeZ); }

void Maze::cleanTrails(int maxStep1, int maxStep2) {
    for (int z = 1; z <= maxZ(); ++z)
        for (int y = 1; y <= maxY(); ++y)
            for (int x = 1; x <= maxX(); ++x) {
                Cell& c = at(x, y, z);
                if (c.step1 > maxStep1) c.step1 = 0;
                if (c.step2 > maxStep2) c.step2 = 0;
            }
}