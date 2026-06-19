#include "renderer.h"
#include <format>

Renderer::Renderer(const Maze& maze) : maze(maze) {}

std::string Renderer::buildFrame(int viewZ, const Player& p1, const Player& p2) const {
    std::string frame;
    const int maxY = maze.maxY();
    const int maxX = maze.maxX();
    const int maxZ = maze.maxZ();

    // 隐藏光标、回到左上角
    frame += "\033[?25l\033[H";

    for (int y = 0; y <= maxY + 1; ++y) {
        for (int x = 0; x <= maxX + 1; ++x) {
            const Cell& cell = maze.at(x, y, viewZ);
            bool p1Here = (p1.getPos().z == viewZ && p1.getPos().x == x && p1.getPos().y == y);
            bool p2Here = (p2.getPos().z == viewZ && p2.getPos().x == x && p2.getPos().y == y);

            if (cell.wall) {
                frame += "\033[43;93m *\033[0m";      // 黄底黄字
            }
            else if (p1Here && p2Here) {
                frame += "\033[46;37m<>\033[0m";      // 双人（青底）
            }
            else if (p1Here) {
                frame += "\033[44;37m<.\033[0m";      // 玩家1（蓝底）
            }
            else if (p2Here) {
                frame += "\033[42;37m.<\033[0m";      // 玩家2（绿底）
            }
            else {
                // 轨迹颜色
                int bg = 0;
                if (cell.step1 < p1.getStep() && cell.step1 != 0) bg = 44;   // 蓝
                if (cell.step2 < p2.getStep() && cell.step2 != 0)
                    bg = (bg == 44) ? 46 : 42;                                // 青或绿
                if (bg)
                    frame += std::format("\033[{};37m  \033[0m", bg);
                else
                    frame += "  ";
            }
        }
        frame += '\n';
    }

    // 终点提示
    int bottomY = maxY + 2;
    bool p1End = (p1.getPos().x == maxX && p1.getPos().y == maxY && p1.getPos().z == maxZ);
    bool p2End = (p2.getPos().x == maxX && p2.getPos().y == maxY && p2.getPos().z == maxZ);

    frame += std::format("\033[{};1H", bottomY);
    if (p1End)
        frame += std::format("\033[44;37mNext...{: <6}\033[0m", maze.at(maxX, maxY, maxZ).step1);

    frame += std::format("\033[{};1H", bottomY + 1);
    if (p2End)
        frame += std::format("\033[42;37mNext...{: <6}\033[0m", maze.at(maxX + 1, maxY, maxZ).step2);

    frame += "\033[?25h";
    return frame;
}