#include "Game.h"
#include "Input.h"
#include "Console.h"
#include <iostream>
#include <format>
#include <chrono>
#include <thread> 

using namespace std::chrono_literals;

Game::Game() : maze(), p1(1, { 1,1,1 }), p2(2, { 2,1,1 }), viewZ(1), renderer(maze) {}

bool Game::init() {
    Console::enableVirtualTerminal();
    if (!maze.load("mg.txt")) {
        std::cout << "Failed to open 'mg.txt'! Program will exit in 5 seconds.\n";
        Sleep(5000);
        return false;
    }
    // 控制台尺寸：每格2字符宽，+1右边距，高度+2提示行
    std::cout << "\033[8;" << maze.maxY() + 4 << ";" << 2 * (maze.maxX() + 2) << ";t" << std::flush;
    Console::setWindowSize(2 * (maze.maxX() + 2) + 1, maze.maxY() + 4);
    return true;
}

void Game::run() {
    while (true) {
        maze.cleanTrails(p1.getStep(), p2.getStep());
        std::string frame = renderer.buildFrame(viewZ, p1, p2);
        std::cout << frame << std::flush;

        p1.backup();
        p2.backup();

        bool exitInput = false;
        while (!exitInput) {
            bool p1Acted = false, p2Acted = false;

            if (auto act = Input::getPlayerAction(1)) {
                if (tryMove(p1, *act)) {
                    p1.recordStep(maze);
                    p1Acted = true;
                }
            }
            if (auto act = Input::getPlayerAction(2)) {
                if (tryMove(p2, *act)) {
                    p2.recordStep(maze);
                    p2Acted = true;
                }
            }

            bool acted = p1Acted || p2Acted;

            if (Input::isViewPrev()) {
                if (viewZ > 1) --viewZ;
                exitInput = true;
            }
            else if (Input::isViewNext()) {
                if (viewZ < maze.maxZ()) ++viewZ;
                exitInput = true;
            }
            else if (acted) {
                if (Input::isAnyDirectionKey())
                    std::this_thread::sleep_for(150ms);  // 防连按
                exitInput = true;
            }
            else {
                break;   // 无输入，结束本帧
            }
        }

        if (checkEndCondition()) break;
        std::this_thread::sleep_for(10ms);
    }
}

bool Game::tryMove(Player& p, Action action) {
    int nx = p.getPos().x, ny = p.getPos().y, nz = p.getPos().z;
    switch (action) {
    case Action::MoveUp:    --ny; break;
    case Action::MoveDown:  ++ny; break;
    case Action::MoveLeft:  --nx; break;
    case Action::MoveRight: ++nx; break;
    case Action::LayerUp:   --nz; break;
    case Action::LayerDown: ++nz; break;
    }
    Player& other = (p.getId() == 1) ? p2 : p1;
    return p.moveTo(nx, ny, nz, maze, other);
}

bool Game::checkEndCondition() const {
    bool p1End = (p1.getPos().x == maze.maxX() && p1.getPos().y == maze.maxY() && p1.getPos().z == maze.maxZ());
    bool p2End = (p2.getPos().x == maze.maxX() && p2.getPos().y == maze.maxY() && p2.getPos().z == maze.maxZ());
    if (p1End && p2End) {
        Sleep(5000);
        return true;
    }
    return false;
}