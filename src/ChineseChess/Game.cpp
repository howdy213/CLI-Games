#include "Game.h"
#include "Console.h"
#include "MessageBar.h"
#include <conio.h>
#include <format>
#include <iostream>
#include <sstream>
#include <windows.h>
#undef SendMessage
namespace ChineseChess {

	void Repeat(std::string_view content, int times) {
		for (int i = 0; i < times; ++i) std::cout << content;
	}

	std::optional<Position> GetMousePos(const std::vector<int>& keyInterrupts) {
		auto& console = Console::Instance();
		auto trans = [](double x, double y) -> Position {
			return { static_cast<int>(((x + 1.0 / 1.5) / 2 + 1) / 2), static_cast<int>(y) };
			};

		while (true) {
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
				POINT mousePos;
				GetCursorPos(&mousePos);
				ScreenToClient(console.GetWindowHandle(), &mousePos);

				Position pos;
				auto& fontInfo = console.GetFontInfo();
				if (fontInfo.dwFontSize.X == 0 || fontInfo.dwFontSize.Y == 0) {
					if (console.isVC) {
						pos = trans((mousePos.x - console.pa.x) /
							static_cast<double>(console.pb.x - console.pa.x) * (37 - 1),
							(mousePos.y - console.pa.y) /
							static_cast<double>(console.pb.y - console.pa.y) * (26 - 1) + 1);
					}
					else {
						std::cerr << "不支持鼠标，前往设置更改";
						return Position{ INT32_MAX, INT32_MAX };
					}
				}
				else {
					pos = trans(mousePos.x / fontInfo.dwFontSize.X, mousePos.y / fontInfo.dwFontSize.Y);
				}
				Sleep(100);
				if (pos.x > 37 / 4 || pos.y > 26 - 1) continue;
				if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
					return pos;
			}
			else if (_kbhit()) {
				int ch = _getch();
				if (std::find(keyInterrupts.begin(), keyInterrupts.end(), ch) != keyInterrupts.end()) {
					Position pos{ INT32_MAX, INT32_MAX };
					pos.keyInterrupt = ch;
					return pos;
				}
			}
		}
	}

	std::optional<Position> GetKeyboardPos(MessageBar& msgBar) {
		std::string input;
		std::getline(std::cin, input);
		std::istringstream iss(input);
		Position pos;
		if (!(iss >> pos.x >> pos.y)) {
			msgBar.AddMessage("输入格式错误，请输入\"x y\"", true);
			return std::nullopt;
		}
		return pos;
	}

	bool ShowMainMenu(GameConfig& config) {
		auto& console = Console::Instance();
		while (true) {
			console.ClearScreen();
			console.Gotoxy(0, 0);
			console.SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);

			auto drawMenuLine = [](int y, std::string_view text) {
				Console::Instance().Gotoxy(0, y);
				std::cout << text;
				};

			drawMenuLine(0, "·----------------------------------·");
			for (int i = 1; i <= 5; ++i) drawMenuLine(i, "|                                    |");
			drawMenuLine(6, "|              中国象棋              |");
			for (int i = 7; i <= 9; ++i) drawMenuLine(i, "|                                    |");
			drawMenuLine(MENU_SETTINGS_Y, "|           >    设置(C)             |");
			drawMenuLine(11, "|                                    |");
			drawMenuLine(MENU_START_Y, "|           >  开始游戏(S)           |");
			drawMenuLine(13, "|                                    |");
			drawMenuLine(MENU_LOAD_Y, "|           >  恢复进度(R)           |");
			drawMenuLine(15, "|                                    |");
			drawMenuLine(MENU_EXIT_Y, "|           >    退出(E)             |");
			for (int i = 17; i <= 22; ++i) drawMenuLine(i, "|                                    |");
			drawMenuLine(23, "|          请不要调整边框位置        |");
			drawMenuLine(24, "|   若为Terminal，配色改为Campbell   |");
			drawMenuLine(25, "·----------------------------------·\n");

			auto menuPos = GetMousePos({ 'c','s','r','e' });
			if (menuPos->keyInterrupt == -1 && menuPos->x == INT_MAX && menuPos->y == INT_MAX)continue;

			if (menuPos->y == MENU_SETTINGS_Y || menuPos->keyInterrupt == 'c') {
				// ---------- 设置子菜单 ----------
				while (true) {
					console.ClearScreen();
					console.Gotoxy(0, 0);
					console.SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);

					// 绘制设置界面头部
					std::cout << "·----------------------------------·\n";
					for (int i = 1; i <= 5; ++i)
						std::cout << "|                                    |\n";
					std::cout << "|                设置                |\n";
					for (int i = 7; i <= 9; ++i)
						std::cout << "|                                    |\n";

					// 控制模式
					console.Gotoxy(0, 10);
					std::cout << "|           >    控制(C)  ";
					switch (config.ctrlMode) {
					case 1: std::cout << "鼠标       |\n"; break;
					case 2: std::cout << "键盘       |\n"; break;
					case 3: std::cout << "鼠键       |\n"; break;
					default: std::cout << "键鼠       |\n"; break;
					}
					std::cout << "|                                    |\n";

					// 规则模式
					console.Gotoxy(0, 12);
					std::cout << "|           >    规则(R)  ";
					std::cout << (config.ruleMode == 1 ? "开启       |\n" : "关闭       |\n");
					std::cout << "|                                    |\n";

					// 窗口模式（系统）
					console.Gotoxy(0, 14);
					std::cout << "|           >    系统(S)   ";
					std::cout << (config.winMode == 1 ? " 7        |\n" : " 10       |\n");
					std::cout << "|                                    |\n";

					// 虚拟终端校准
					console.Gotoxy(0, 16);
					std::cout << "|           >    虚拟终端(A)         |\n";
					std::cout << "|                触点校准            |\n";
					std::cout << "|                                    |\n";

					// 空白行和退出
					console.Gotoxy(0, 19);
					std::cout << "|           >    退出(Q)             |\n";
					for (int i = 20; i <= 22; ++i) {
						console.Gotoxy(0, i);
						std::cout << "|                                    |\n";
					}
					std::cout << "|          请不要调整边框位置        |\n";
					std::cout << "|                                    |\n";
					std::cout << "·----------------------------------·\n";

					// 等待输入
					auto setPos = GetMousePos({ 'c','r','s','q','a' });
					if (!setPos) continue;

					// 处理操作
					if (setPos->y == 10 || setPos->keyInterrupt == 'c') {
						config.ctrlMode = (config.ctrlMode % 4) + 1; // 1→2→3→4→1
					}
					else if (setPos->y == 12 || setPos->keyInterrupt == 'r') {
						config.ruleMode = (config.ruleMode % 2) + 1; // 1↔2
					}
					else if (setPos->y == 14 || setPos->keyInterrupt == 's') {
						config.winMode = (config.winMode % 2) + 1;   // 1↔2
					}
					else if (setPos->y == 16 || setPos->keyInterrupt == 'a') {
						console.AdjustPointForVirtualConsole(37, 26); // 虚拟终端校准
					}
					else if (setPos->y == 19 || setPos->keyInterrupt == 'q') {
						break; // 退出设置
					}

					Sleep(80); // 避免快速连击
				}
				continue; // 确保重新绘制主菜单
			}
			else if (menuPos->y == MENU_START_Y || menuPos->keyInterrupt == 's')
				return true;  // 新游戏
			else if (menuPos->y == MENU_LOAD_Y || menuPos->keyInterrupt == 'r')
				return false; // 加载存档
			else if (menuPos->y == MENU_EXIT_Y || menuPos->keyInterrupt == 'e')
				std::exit(0);
		}
	}

	void GameLoop(const GameConfig& config, bool isNewGame) {
		ChessBoard board(config.winMode, isNewGame ? "" : "savegame.dat");
		auto& console = Console::Instance();
		console.ClearScreen();

		bool selectingFrom = true;
		Position from, to;

		while (!board.IsGameOver()) {
			board.Display();
			auto& msg = board.GetMessageBar();
			std::string prompt = std::format("{}输入坐标{}(x,y)",
				(config.ctrlMode == 1 || (config.ctrlMode == 3 && board.GetCurrentPlayer() == PieceColor::RED) ||
					(config.ctrlMode == 4 && board.GetCurrentPlayer() == PieceColor::BLACK)) ? "鼠标" : "键盘",
				selectingFrom ? 1 : 2);
			msg.SendMessage(prompt);

			std::optional<Position> optPos;
			bool useMouse = (config.ctrlMode == 1) ||
				(config.ctrlMode == 3 && board.GetCurrentPlayer() == PieceColor::RED) ||
				(config.ctrlMode == 4 && board.GetCurrentPlayer() == PieceColor::BLACK);
			if (useMouse) {
				optPos = GetMousePos();
			}
			else {
				console.Gotoxy(3, 22);
				console.SetColor(ConsoleColor::YELLOW,
					board.GetCurrentPlayer() == PieceColor::BLACK ? ConsoleColor::BLACK : ConsoleColor::RED);
				std::cout << "第" << (selectingFrom ? 1 : 2) << "个:                             ";
				console.Gotoxy(9, 22);
				optPos = GetKeyboardPos(msg);
			}

			if (!optPos) continue;
			Position trans = optPos.value();

			// 仅鼠标输入需要从文本坐标转换为棋盘数组索引
			if (useMouse) {
				trans.x -= 1;
				trans.y = trans.y / 2 - 1;
			}

			if (trans.x < -1 || trans.x >= BOARD_COLS || trans.y < -1 || trans.y >= BOARD_ROWS) {
				msg.AddMessage("位置输入错误（超出范围）", true);
				continue;
			}

			if (trans.y == -1) {
				if (trans.x >= -1 && trans.x <= 2) board.SaveGame();
				else if (trans.x >= 7 && trans.x <= 9) return;
				else msg.AddMessage("无效操作", true);
				continue;
			}

			if (selectingFrom) {
				from = trans;
				msg.AddMessage(std::format("已选择位置: ({},{})", from.x, from.y));
			}
			else {
				to = trans;
				board.MovePiece(from, to, config.ruleMode == 1);
			}
			selectingFrom = !selectingFrom;
			Sleep(50);
		}

		board.Display();
		board.GetMessageBar().AddMessage(
			std::format("游戏结束！{}获胜！", board.GetWinner() == PieceColor::RED ? "红方" : "黑方"));
		board.GetMessageBar().Display();
		Sleep(3000);
	}

} // namespace ChineseChess