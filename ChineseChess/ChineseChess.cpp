#include <conio.h>
#include <cstdlib>
#include <fstream>
#include <io.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
using namespace std;

// 全局工具函数：重复输出字符串
void repeat(const string& content, int times) {
	for (int i = 0; i < times; ++i) {
		cout << content;
	}
}

namespace ChineseChess {
	// 常量定义：棋盘参数
	const int BOARD_ROWS = 10;    // 棋盘行数
	const int BOARD_COLS = 9;     // 棋盘列数
	const int RED_GENERAL_ROW_MIN = 7;  // 红帅九宫行范围
	const int RED_GENERAL_ROW_MAX = 9;
	const int RED_GENERAL_COL_MIN = 3;  // 红帅九宫列范围
	const int RED_GENERAL_COL_MAX = 5;
	const int BLACK_GENERAL_ROW_MIN = 0; // 黑将九宫行范围
	const int BLACK_GENERAL_ROW_MAX = 2;
	const int BLACK_GENERAL_COL_MIN = 3; // 黑将九宫列范围
	const int BLACK_GENERAL_COL_MAX = 5;
	const int RIVER_ROW = 4;      // 楚河汉界所在行

	// 界面常量
	const int MSG_BAR_TOP_ROW = 24;    // 消息栏起始行
	const int MSG_BAR_MAX_LINES = 3;   // 消息栏最大行数
	const int MENU_SETTINGS_Y = 10;    // 主菜单"设置"项行坐标
	const int MENU_START_Y = 12;       // 主菜单"开始游戏"项行坐标
	const int MENU_LOAD_Y = 14;        // 主菜单"恢复进度"项行坐标
	const int MENU_EXIT_Y = 16;        // 主菜单"退出"项行坐标

	// 颜色枚举（背景色+前景色）
	enum class ConsoleColor {
		BLACK = 0,
		RED = 4,
		YELLOW = 14
	};

	// 控制台工具类（封装控制台操作）
	class Console {
	public:
		bool isVC = false;
		POINT pa = {};
		POINT pb = {};
		POINT pt = {};
		POINT rt = { 37,26 };
	private:
		HWND window = nullptr;                    // 控制台窗口句柄
		HANDLE outputHandle = nullptr;            // 输出句柄
		CONSOLE_FONT_INFO fontInfo = {};     // 字体信息
		CONSOLE_CURSOR_INFO cursorInfo = {}; // 光标信息

		void setupConsoleWindow() {
			HWND consoleWindow = GetConsoleWindow();//禁止缩放
			SetWindowLong(consoleWindow, GWL_STYLE,
				GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

			SMALL_RECT windowRect = { 0, 0, rt.x, rt.y };//窗口大小
			CONSOLE_SCREEN_BUFFER_INFO bufferInfo;//去除滚动条
			GetConsoleScreenBufferInfo(outputHandle, &bufferInfo);
			SMALL_RECT infoRect = windowRect;// bufferInfo.srWindow;
			COORD bufferSize = { infoRect.Right + 1, infoRect.Bottom + 1 };
			SetConsoleScreenBufferSize(outputHandle, bufferSize);

			SetConsoleWindowInfo(outputHandle, true, &windowRect);
		}

		void setupConsoleMode() {
			HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
			DWORD mode;
			GetConsoleMode(inputHandle, &mode);
			mode &= ~ENABLE_QUICK_EDIT_MODE; // 禁用快速编辑模式
			SetConsoleMode(inputHandle, mode);
		}

	public:
		Console() : window(nullptr), outputHandle(nullptr) {}

		void initialize() {
			window = GetForegroundWindow();
			outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			GetCurrentConsoleFont(outputHandle, 0, &fontInfo);
			setupConsoleWindow();
			setupConsoleMode();

			GetConsoleCursorInfo(outputHandle, &cursorInfo);// 隐藏光标
			cursorInfo.bVisible = false;
			SetConsoleCursorInfo(outputHandle, &cursorInfo);
		}

		void gotoxy(short x, short y) const {
			COORD coord = { x, y };
			SetConsoleCursorPosition(outputHandle, coord);
		}

		void setColor(ConsoleColor background, ConsoleColor foreground) const {
			SetConsoleTextAttribute(outputHandle,
				FOREGROUND_INTENSITY | BACKGROUND_INTENSITY |
				static_cast<int>(background) * 16 +
				static_cast<int>(foreground));
		}

		void clearScreen() const {
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			DWORD written;
			COORD topLeft = { 0, 0 };

			GetConsoleScreenBufferInfo(outputHandle, &csbi);
			FillConsoleOutputCharacter(outputHandle, L' ',
				csbi.dwSize.X * csbi.dwSize.Y,
				topLeft, &written);
			FillConsoleOutputAttribute(outputHandle,
				static_cast<WORD>(ConsoleColor::YELLOW) * 16 +
				static_cast<WORD>(ConsoleColor::BLACK),
				csbi.dwSize.X * csbi.dwSize.Y,
				topLeft, &written);
			gotoxy(0, 0);
		}

		void AdjustPointForVirtualConsole(int x, int y) {
			isVC = true;
			pt.x = x;
			pt.y = y;
			auto getPos = [&](POINT& p) {
				while (1) {
					if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
						GetCursorPos(&p);
						ScreenToClient(getWindowHandle(), &p);
						Sleep(100);
						if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) break;
					}
				}
				};
			gotoxy(0, 0);
			setColor(ConsoleColor::YELLOW, ConsoleColor::RED);
			cout << "@";
			setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
			cout << "点击";
			getPos(pa);
			gotoxy(x - 5, y);
			setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
			cout << "点击";
			setColor(ConsoleColor::YELLOW, ConsoleColor::RED);
			cout << "@";
			getPos(pb);
		}
		// 获取字体信息（供坐标转换使用）
		const CONSOLE_FONT_INFO& getFontInfo() const { return fontInfo; }
		HWND getWindowHandle() const { return window; }
	};

	Console console;

	// 消息栏类（底部固定区域，用于显示提示信息）
	class MessageBar {
	private:
		vector<string> messages;  // 消息存储容器
		int maxLines;             // 最大显示行数
		int barTopRow;            // 消息栏起始行位置

	public:
		MessageBar(int topRow, int lines = 3) : barTopRow(topRow), maxLines(lines) { messages.reserve(maxLines); }

		// 添加消息（支持不同类型：普通/错误）
		void addMessage(const string& text, bool isError = false) {
			static int idx = 0;
			idx = (idx + 1) % 10;
			string msg = to_string(idx) + (isError ? "[错误] " : "[提示] ") + text;

			// 超过最大行数时移除最旧消息
			if (messages.size() >= maxLines) {
				messages.erase(messages.begin());
			}
			messages.push_back(msg);
		}
		void sendMessage(const string& text, bool isError = false) {
			addMessage(text, isError);
			display();
		}
		void clear() { messages.clear(); }

		// 显示消息栏（固定在底部）
		void display() const {
			// 绘制消息栏分隔线
			console.gotoxy(0, barTopRow - 1);
			console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
			repeat("─", 39);

			// 显示消息
			for (int i = 0; i < maxLines; ++i) {
				console.gotoxy(0, barTopRow + i);
				if (i < messages.size()) {
					// 错误消息显示红色文字，普通消息黑色
					if (messages[i].find("[错误]") != string::npos) {
						console.setColor(ConsoleColor::YELLOW, ConsoleColor::RED);
					}
					else {
						console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
					}
					cout << messages[i];
				}
				// 填充空白覆盖旧消息
				repeat(" ", 39 - (i < messages.size() ? (int)messages[i].size() : 0));
			}
			console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
		}
	};

	// 位置结构体（x表示列，y表示行）
	struct Position {
		int x, y; // x:列, y:行
		int keyInterrupt = -1;
		Position(int x = 0, int y = 0) : x(x), y(y) {}
	};

	// 获取鼠标点击的棋盘位置
	Position GetMousePos(vector<int> KeyInterrrupt = {}) {
		Position pos;
		auto trans = [](double x, double y) {
			return Position{
				(int)((((x + 1 / 1.5) / 2) + 1) / 2),
				(int)y
			};
			};
		while (true) {
			POINT mousePos;
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
				GetCursorPos(&mousePos);
				ScreenToClient(console.getWindowHandle(), &mousePos);

				// 坐标转换逻辑：将鼠标像素坐标转换为棋盘格子坐标
				auto& fontInfo = console.getFontInfo();
				if (fontInfo.dwFontSize.X == 0 || fontInfo.dwFontSize.Y == 0) {
					if (console.isVC) {
						pos = trans((mousePos.x - console.pa.x) / (double)(console.pb.x - console.pa.x) * (console.pt.x - 1),
							(mousePos.y - console.pa.y) / (double)(console.pb.y - console.pa.y) * (console.pt.y - 1) + 1);
					}
					else {
						cerr << "不支持鼠标,前往设置更改";
						pos = { INT32_MAX,INT32_MAX };
						return pos;
					}
				}
				else pos = trans((mousePos.x / fontInfo.dwFontSize.X), mousePos.y / fontInfo.dwFontSize.Y);
				Sleep(100);
				if (pos.x > console.rt.x / 4 || pos.y > console.rt.y - 1)continue;
				if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) break;
			}
			else {
				if (_kbhit()) {
					int response = _getch();
					for (auto it = KeyInterrrupt.begin(); it != KeyInterrrupt.end(); it++) {
						if (*it == response) {
							pos = { INT32_MAX,INT32_MAX };
							pos.keyInterrupt = response;
							return pos;
						}
					}
				}
			}
		}
		return pos;
	}

	// 获取键盘输入的坐标
	Position GetKeyboardPos(MessageBar& msgBar) {
		Position pos;
		string input;
		getline(cin, input);
		istringstream iss(input);
		if (!(iss >> pos.x >> pos.y)) {
			pos.x = INT32_MAX;
			pos.y = INT32_MAX;
			msgBar.addMessage("输入格式错误，请输入\"x y\"", true);
		}
		return pos;
	}

	// 棋子基类
	class Piece {
	public:
		enum Type {
			EMPTY,
			// 红方棋子
			R_GENERAL, R_ADVISOR, R_ELEPHANT, R_HORSE, R_CHARIOT, R_CANNON, R_SOLDIER,
			// 黑方棋子
			B_GENERAL, B_ADVISOR, B_ELEPHANT, B_HORSE, B_CHARIOT, B_CANNON, B_SOLDIER
		};

		enum Color {
			RED,    // 红方
			BLACK,  // 黑方
			NONE    // 无颜色（空位置）
		};

		Type type;    // 棋子类型
		Color color;  // 颜色
		bool alive;   // 是否存活

		Piece(Type type = EMPTY, Color color = NONE)
			: type(type), color(color), alive(false) {
		}

		virtual ~Piece() = default;

		// 验证移动是否有效
		virtual bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const = 0;

		// 获取棋子的中文字符
		string getChineseChar() const {
			static const string pieceChars[] = {
				"　", // 空
				"帅", "仕", "相", "马", "车", "炮", "兵", // 红方
				"将", "士", "象", "马", "车", "炮", "卒"  // 黑方
			};
			return pieceChars[type];
		}

		Color getColor() const { return color; }

	protected:
		// 通用检查：不能吃己方棋子
		bool isSameColor(const vector<vector<shared_ptr<Piece>>>& board, Position to) const {
			return board[to.y][to.x] && board[to.y][to.x]->color == color;
		}
	};

	// 棋子工厂函数声明
	shared_ptr<Piece> PieceFactory(Piece::Type type, Piece::Color color);

	// 车类
	class Chariot : public Piece {
	public:
		Chariot(Color color) : Piece(color == RED ? R_CHARIOT : B_CHARIOT, color) {
			alive = true;
		}

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// 不能吃己方棋子
			if (isSameColor(board, to)) return false;

			// 必须直线移动
			if (from.x != to.x && from.y != to.y) return false;

			// 检查路径是否有障碍物
			if (from.x == to.x) { // 垂直移动（列相同）
				int step = (from.y < to.y) ? 1 : -1;
				for (int y = from.y + step; y != to.y; y += step) {
					if (board[y][from.x] != nullptr) return false;
				}
			}
			else { // 水平移动（行相同）
				int step = (from.x < to.x) ? 1 : -1;
				for (int x = from.x + step; x != to.x; x += step) {
					if (board[from.y][x] != nullptr) return false;
				}
			}

			return true;
		}
	};

	// 马类
	class Horse : public Piece {
	public:
		Horse(Color color) : Piece(color == RED ? R_HORSE : B_HORSE, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// 不能吃己方棋子
			if (isSameColor(board, to)) return false;

			// 马走"日"字
			int dx = abs(from.x - to.x);
			int dy = abs(from.y - to.y);
			if (!((dx == 1 && dy == 2) || (dx == 2 && dy == 1))) return false;

			// 检查马腿是否被绊
			Position legPos = from;
			if (dx == 2) legPos.x += (to.x > from.x) ? 1 : -1; // 水平方向移动
			else legPos.y += (to.y > from.y) ? 1 : -1; // 垂直方向移动

			return board[legPos.y][legPos.x] == nullptr;
		}
	};

	// 象/相类
	class Elephant : public Piece {
	public:
		Elephant(Color color) : Piece(color == RED ? R_ELEPHANT : B_ELEPHANT, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// 不能吃己方棋子
			if (isSameColor(board, to)) return false;

			// 象走"田"字
			int dx = abs(from.x - to.x), dy = abs(from.y - to.y);
			if (dx != 2 || dy != 2) return false;

			// 检查象眼是否被塞
			Position eyePos((from.x + to.x) / 2, (from.y + to.y) / 2);
			if (board[eyePos.y][eyePos.x] != nullptr) return false;

			// 象不能过河
			if ((color == RED && to.y < 5) || (color == BLACK && to.y > 4)) return false;

			return true;
		}
	};

	// 士/仕类
	class Advisor : public Piece {
	public:
		Advisor(Color color) : Piece(color == RED ? R_ADVISOR : B_ADVISOR, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// 不能吃己方棋子
			if (isSameColor(board, to)) return false;

			// 士走斜线一步
			int dx = abs(from.x - to.x), dy = abs(from.y - to.y);
			if (dx != 1 || dy != 1) return false;

			// 不能出九宫
			if (color == RED) {
				if (to.y < RED_GENERAL_ROW_MIN || to.y > RED_GENERAL_ROW_MAX ||
					to.x < RED_GENERAL_COL_MIN || to.x > RED_GENERAL_COL_MAX) {
					return false;
				}
			}
			else {
				if (to.y < BLACK_GENERAL_ROW_MIN || to.y > BLACK_GENERAL_ROW_MAX ||
					to.x < BLACK_GENERAL_COL_MIN || to.x > BLACK_GENERAL_COL_MAX) {
					return false;
				}
			}
			return true;
		}
	};

	// 将/帅类
	class General : public Piece {
	public:
		General(Color color) : Piece(color == RED ? R_GENERAL : B_GENERAL, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// 不能吃己方棋子
			if (isSameColor(board, to)) return false;

			// 特殊情况：将帅对面（飞将）
			if (board[to.y][to.x] &&
				(board[to.y][to.x]->type == Piece::B_GENERAL ||
					board[to.y][to.x]->type == Piece::R_GENERAL) &&
				to.x == from.x) {

				int minY = min(to.y, from.y);
				int maxY = max(to.y, from.y);
				for (int y = minY + 1; y < maxY; ++y) {
					if (board[y][to.x] != nullptr) {
						return false; // 中间有棋子阻挡
					}
				}
				return true;
			}

			// 正常移动：只能走一步直线
			int dx = abs(from.x - to.x), dy = abs(from.y - to.y);
			if ((dx + dy) != 1) return false;

			// 不能出九宫
			if (color == RED) {
				if (to.y < RED_GENERAL_ROW_MIN || to.y > RED_GENERAL_ROW_MAX ||
					to.x < RED_GENERAL_COL_MIN || to.x > RED_GENERAL_COL_MAX) {
					return false;
				}
			}
			else {
				if (to.y < BLACK_GENERAL_ROW_MIN || to.y > BLACK_GENERAL_ROW_MAX ||
					to.x < BLACK_GENERAL_COL_MIN || to.x > BLACK_GENERAL_COL_MAX) {
					return false;
				}
			}

			return true;
		}
	};

	// 炮类
	class Cannon : public Piece {
	public:
		Cannon(Color color) : Piece(color == RED ? R_CANNON : B_CANNON, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// 不能吃己方棋子
			if (isSameColor(board, to)) return false;

			// 必须直线移动
			if (from.x != to.x && from.y != to.y) return false;

			// 计算路径上的障碍物数量
			int obstacleCount = 0;
			if (from.x == to.x) { // 垂直移动
				int step = (from.y < to.y) ? 1 : -1;
				for (int y = from.y + step; y != to.y; y += step) {
					if (board[y][from.x] != nullptr) obstacleCount++;
				}
			}
			else { // 水平移动
				int step = (from.x < to.x) ? 1 : -1;
				for (int x = from.x + step; x != to.x; x += step) {
					if (board[from.y][x] != nullptr) obstacleCount++;
				}
			}

			// 炮翻山规则：吃子需一个障碍物，移动需0个
			bool isCapturing = (board[to.y][to.x] != nullptr);
			return (isCapturing && obstacleCount == 1) || (!isCapturing && obstacleCount == 0);
		}
	};

	// 士兵/卒类
	class Soldier : public Piece {
	public:
		Soldier(Color color) : Piece(color == RED ? R_SOLDIER : B_SOLDIER, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// 不能吃己方棋子
			if (isSameColor(board, to)) return false;

			// 移动范围检查：只能走一步
			int dx = abs(from.x - to.x);
			int dy = to.y - from.y;
			if (dx > 1 || abs(dy) > 1 || (dx == 1 && dy != 0)) return false;

			// 前进方向检查（红兵向上，黑兵向下）
			if ((color == RED && dy > 0) || (color == BLACK && dy < 0)) return false;

			// 未过河不能横向移动
			if (dx == 1 && ((color == RED && from.y > 4) || (color == BLACK && from.y < 5))) return false;

			return true;
		}
	};

	// 棋子工厂实现
	shared_ptr<Piece> PieceFactory(Piece::Type type, Piece::Color color) {
		switch (type) {
		case Piece::R_GENERAL: case Piece::B_GENERAL:
			return make_shared<General>(color);
		case Piece::R_ADVISOR: case Piece::B_ADVISOR:
			return make_shared<Advisor>(color);
		case Piece::R_ELEPHANT: case Piece::B_ELEPHANT:
			return make_shared<Elephant>(color);
		case Piece::R_HORSE: case Piece::B_HORSE:
			return make_shared<Horse>(color);
		case Piece::R_CHARIOT: case Piece::B_CHARIOT:
			return make_shared<Chariot>(color);
		case Piece::R_CANNON: case Piece::B_CANNON:
			return make_shared<Cannon>(color);
		case Piece::R_SOLDIER: case Piece::B_SOLDIER:
			return make_shared<Soldier>(color);
		default:
			return nullptr;
		}
	}

	// 存档管理器
	class SaveManager {
	public:
		// 保存游戏状态到文件
		static bool saveGame(const vector<vector<shared_ptr<Piece>>>& board,
			Piece::Color currentPlayer, const string& filename) {
			ofstream file(filename);
			if (!file.is_open()) return false;

			// 保存当前玩家
			file << (currentPlayer == Piece::RED ? "RED" : "BLACK") << endl;

			// 保存棋盘状态
			for (int y = 0; y < BOARD_ROWS; ++y) {
				for (int x = 0; x < BOARD_COLS; ++x) {
					file << (board[y][x] ? static_cast<int>(board[y][x]->type) : 0) << " ";
				}
				file << endl;
			}
			return true;
		}

		// 从文件加载游戏状态
		static bool loadGame(vector<vector<shared_ptr<Piece>>>& board,
			Piece::Color& currentPlayer, const string& filename) {
			ifstream file(filename);
			if (!file.is_open()) return false;

			// 初始化棋盘
			board = vector<vector<shared_ptr<Piece>>>(BOARD_ROWS,
				vector<shared_ptr<Piece>>(BOARD_COLS, nullptr));

			// 读取当前玩家
			string playerStr;
			file >> playerStr;
			currentPlayer = (playerStr == "RED") ? Piece::RED : Piece::BLACK;

			// 读取棋盘状态
			for (int y = 0; y < BOARD_ROWS; ++y) {
				for (int x = 0; x < BOARD_COLS; ++x) {
					int pieceType;
					file >> pieceType;
					if (pieceType != 0) {
						Piece::Type type = static_cast<Piece::Type>(pieceType);
						Piece::Color color = (pieceType >= Piece::R_GENERAL && pieceType <= Piece::R_SOLDIER)
							? Piece::RED : Piece::BLACK;
						board[y][x] = PieceFactory(type, color);
					}
				}
			}

			return true;
		}
	};

	// 棋盘类
	class ChessBoard {
	private:
		vector<vector<shared_ptr<Piece>>> board; // 10行9列棋盘（board[y][x]）
		Piece::Color currentPlayer;
		bool gameOver;
		Piece::Color winner;
		int winMode;
		MessageBar msgBar;  // 消息栏成员

		// 放置一方棋子的通用函数（减少重复代码）
		void placePieces(Piece::Color color, int backRow, int cannonRow, int soldierRow) {
			// 放置车、马、象、士、将
			board[backRow][0] = make_shared<Chariot>(color);
			board[backRow][1] = make_shared<Horse>(color);
			board[backRow][2] = make_shared<Elephant>(color);
			board[backRow][3] = make_shared<Advisor>(color);
			board[backRow][4] = make_shared<General>(color);
			board[backRow][5] = make_shared<Advisor>(color);
			board[backRow][6] = make_shared<Elephant>(color);
			board[backRow][7] = make_shared<Horse>(color);
			board[backRow][8] = make_shared<Chariot>(color);

			// 放置炮
			board[cannonRow][1] = make_shared<Cannon>(color);
			board[cannonRow][7] = make_shared<Cannon>(color);

			// 放置兵/卒
			board[soldierRow][0] = make_shared<Soldier>(color);
			board[soldierRow][2] = make_shared<Soldier>(color);
			board[soldierRow][4] = make_shared<Soldier>(color);
			board[soldierRow][6] = make_shared<Soldier>(color);
			board[soldierRow][8] = make_shared<Soldier>(color);
		}

		// 检查游戏是否结束（将/帅是否被吃）
		void checkGameOver() {
			bool redAlive = false, blackAlive = false;
			for (int y = 0; y < BOARD_ROWS; ++y) {
				for (int x = 0; x < BOARD_COLS; ++x) {
					if (board[y][x]) {
						if (board[y][x]->type == Piece::R_GENERAL) redAlive = true;
						if (board[y][x]->type == Piece::B_GENERAL) blackAlive = true;
					}
				}
			}

			if (!redAlive) {
				gameOver = true;
				winner = Piece::BLACK;
			}
			else if (!blackAlive) {
				gameOver = true;
				winner = Piece::RED;
			}
		}

	public:
		ChessBoard(int winMode) : board(BOARD_ROWS, vector<shared_ptr<Piece>>(BOARD_COLS, nullptr)),
			currentPlayer(Piece::RED), gameOver(false), winner(Piece::NONE),
			winMode(winMode), msgBar(MSG_BAR_TOP_ROW, MSG_BAR_MAX_LINES) {
			initializeBoard();
			msgBar.addMessage("游戏开始！红方先行，点击棋子选择移动");
		}

		// 从存档加载
		ChessBoard(int winMode, const string& filename) : board(BOARD_ROWS, vector<shared_ptr<Piece>>(BOARD_COLS, nullptr)),
			winMode(winMode), msgBar(MSG_BAR_TOP_ROW, MSG_BAR_MAX_LINES) {
			if (SaveManager::loadGame(board, currentPlayer, filename)) {
				gameOver = false;
				winner = Piece::NONE;
				msgBar.addMessage("(已加载存档)当前回合: " + string(currentPlayer == Piece::RED ? "红方" : "黑方"));
			}
			else {
				initializeBoard();
				currentPlayer = Piece::RED;
				gameOver = false;
				winner = Piece::NONE;
				msgBar.addMessage("(无存档)新游戏:红方先行");
			}
		}

		// 获取消息栏（供外部添加消息）
		MessageBar& getMsgBar() { return msgBar; }

		// 初始化棋盘
		void initializeBoard() {
			placePieces(Piece::BLACK, 0, 2, 3); // 黑方棋子（后排0，炮2，卒3）
			placePieces(Piece::RED, 9, 7, 6);   // 红方棋子（后排9，炮7，兵6）
		}

		// 显示棋盘
		void displayBoard() const {
			//console.clearScreen();
			console.gotoxy(0, 0);
			console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
			cout << "保存                             返回\n";
			cout << "   0   1   2   3   4   5   6   7   8\n";

			for (int y = 0; y < BOARD_ROWS; ++y) {
				console.gotoxy(0, 2 * y + 2);
				console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
				cout << " " << y;

				for (int x = 0; x < BOARD_COLS; ++x) {
					cout << " ";
					auto piece = board[y][x];
					if (piece) {
						// 设置棋子颜色：红方(黄底红) 黑方(黄底黑)
						console.setColor(ConsoleColor::YELLOW,
							piece->color == Piece::RED ? ConsoleColor::RED : ConsoleColor::BLACK);
						cout << piece->getChineseChar();
						console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
					}
					else {
						// 绘制棋盘格线
						drawGrid(y, x);
					}
					if (x != 8) cout << "─";
				}

				cout << " \n";
				console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);

				// 绘制九宫格斜线和楚河汉界
				drawSpecialLines(y + 2);
			}

			// 显示当前回合玩家
			console.gotoxy(0, MSG_BAR_TOP_ROW - 2);
			console.setColor(ConsoleColor::YELLOW,
				currentPlayer ? ConsoleColor::BLACK : ConsoleColor::RED);
			cout << (currentPlayer ? "黑" : "红") << endl;

			// 显示消息栏
			msgBar.display();
		}

		// 绘制棋盘网格
		void drawGrid(int y, int x) const {
			if (y == 0 && x == 0) cout << "┌─";
			else if (y == 0 && x == 8) cout << "┐";
			else if (y == 9 && x == 0) cout << "└─";
			else if (y == 9 && x == 8) cout << "┘";
			else if (x == 0) cout << "├─";
			else if (x == 8) cout << "┤";
			else if ((y == 9 || y == 4) && x < 8) cout << "┴─";
			else if ((y == 0 || y == 5) && x < 8) cout << "┬─";
			else cout << "┼─";
		}

		// 绘制特殊线条（九宫格斜线和楚河汉界）
		void drawSpecialLines(int row) const {
			switch (row) {
			case 2: // 黑方九宫斜线
				if (winMode == 1) cout << "  │  │  │  │╲│╱│  │  │  │ \n";
				else cout << "   │   │   │   │ ╲ │ ╱ │   │   │   │ \n";
				break;
			case 3: // 黑方九宫斜线
				if (winMode == 1) cout << "  │  │  │  │╱│╲│  │  │  │ \n";
				else cout << "   │   │   │   │ ╱ │ ╲ │   │   │   │ \n";
				break;
			case 6: // 楚河汉界
				if (winMode == 1) cout << "  │       楚河       汉界        │ \n";
				else cout << "   │        楚河       汉界        │ \n";
				break;
			case 9: // 红方九宫斜线
				if (winMode == 1) cout << "  │  │  │  │╲│╱│  │  │  │ \n";
				else cout << "   │   │   │   │ ╲ │ ╱ │   │   │   │ \n";
				break;
			case 10: // 红方九宫斜线
				if (winMode == 1) cout << "  │  │  │  │╱│╲│  │  │  │ \n";
				else cout << "   │   │   │   │ ╱ │ ╲ │   │   │   │ \n";
				break;
			default: // 普通横线
				if (row >= 11)return;
				if (winMode == 1) cout << "  │  │  │  │  │  │  │  │  │ \n";
				else cout << "   │   │   │   │   │   │   │   │   │ \n";
				break;
			}
		}

		// 移动棋子
		bool movePiece(Position from, Position to, bool enforceRules = true) {
			// 检查起始位置是否有棋子
			if (!board[from.y][from.x]) {
				msgBar.addMessage("起始位置没有棋子！", true);
				return false;
			}

			// 检查是否选择己方棋子
			if (board[from.y][from.x]->color != currentPlayer) {
				msgBar.addMessage("请选择" + string(currentPlayer == Piece::RED ? "红" : "黑") + "方的棋子！", true);
				return false;
			}

			// 检查移动是否符合规则
			if (enforceRules && !board[from.y][from.x]->isValidMove(board, from, to)) {
				msgBar.addMessage("此移动不符合棋子规则！", true);
				return false;
			}

			// 移动成功提示
			string pieceName = board[from.y][from.x]->getChineseChar();
			msgBar.addMessage("已移动" + pieceName + "：(" + to_string(from.x) + "," + to_string(from.y) +
				") → (" + to_string(to.x) + "," + to_string(to.y) + ")");

			// 执行移动
			board[to.y][to.x] = board[from.y][from.x];
			board[from.y][from.x] = nullptr;

			checkGameOver(); // 检查游戏是否结束
			// 切换玩家
			currentPlayer = (currentPlayer == Piece::RED) ? Piece::BLACK : Piece::RED;
			return true;
		}

		Piece::Color getCurrentPlayer() const { return currentPlayer; }

		// 保存游戏
		bool saveGame(const string& filename = "savegame.dat") {
			bool result = SaveManager::saveGame(board, currentPlayer, filename);
			if (result) msgBar.addMessage("游戏已保存至 " + filename);
			else msgBar.addMessage("保存失败！", true);
			return result;
		}

		bool isGameOver() const { return gameOver; }
		Piece::Color getWinner() const { return winner; }
	};

	// 游戏配置
	struct GameConfig {
		int ctrlMode = 1;   // 控制模式（1:鼠标 2:键盘 3:鼠键 4:键鼠）
		int ruleMode = 1;   // 规则模式（1:开启 2:关闭）
		int winMode = 2;    // 窗口模式（1:≤7 2:≥10）
	};

	// 显示主菜单
	bool showMainMenu(GameConfig& config) {
		bool started = false;
		while (!started) {
			console.clearScreen();
			console.gotoxy(0, 0);

			console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
			cout << "·----------------------------------" << (console.isVC ? "--" : "") << "·\n";
			repeat("|                                    |\n", 5);
			cout << "|              中国象棋              |\n";
			repeat("|                                    |\n", 3);
			cout << "|           >    设置(C)             |\n";
			repeat("|                                    |\n", 1);
			cout << "|           >  开始游戏(S)           |\n";
			repeat("|                                    |\n", 1);
			cout << "|           >  恢复进度(R)           |\n";
			repeat("|                                    |\n", 1);
			cout << "|           >    退出(E)             |\n";
			repeat("|                                    |\n", 3);
			cout << "|          请不要调整边框位置        |\n";
			cout << "|   若为虚拟终端，配色改为Campbell   |\n";
			repeat("|                                    |\n", 1);
			cout << "·----------------------------------" << (console.isVC ? "--" : "") << "·\n";

			Position menuPos = GetMousePos({ 'c','s','r','e' });
			if (menuPos.y == MENU_SETTINGS_Y || menuPos.keyInterrupt == 'c') {
				// 显示设置菜单
				while (true) {
					console.clearScreen();
					console.gotoxy(0, 0);

					console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
					cout << "·----------------------------------" << (console.isVC ? "--" : "") << "·\n";
					repeat("|                                    |\n", 5);
					cout << "|                设置                |\n";
					repeat("|                                    |\n", 3);
					cout << "|           >    控制(C)  ";
					switch (config.ctrlMode) {
					case 1: cout << "鼠标       |\n"; break;
					case 2: cout << "键盘       |\n"; break;
					case 3: cout << "鼠键       |\n"; break;
					default: cout << "键鼠       |\n"; break;
					}
					repeat("|                                    |\n", 1);
					cout << "|           >    规则(R)  ";
					cout << (config.ruleMode == 1 ? "开启       |\n" : "关闭       |\n");
					repeat("|                                    |\n", 1);
					cout << "|           >    系统(S)   ";
					cout << (config.winMode == 1 ? " 7        |\n" : " 10       |\n");
					repeat("|                                    |\n", 1);
					cout << "|           >    虚拟终端(A)         |\n";
					cout << "|                触点校准            |\n";
					repeat("|                                    |\n", 1);
					cout << "|           >    退出(Q)             |\n";
					repeat("|                                    |\n", 2);
					cout << "|          请不要调整边框位置        |\n";
					cout << "·----------------------------------" << (console.isVC ? "--" : "") << "·\n";

					Position setPos = GetMousePos({ 'c','r','s','q' ,'a' });
					if (setPos.y == 10 || setPos.keyInterrupt == 'c') config.ctrlMode = (config.ctrlMode % 4) + 1; // 循环切换1-4
					else if (setPos.y == 12 || setPos.keyInterrupt == 'r') config.ruleMode = (config.ruleMode % 2) + 1; // 循环切换1-2
					else if (setPos.y == 14 || setPos.keyInterrupt == 's') config.winMode = (config.winMode % 2) + 1;   // 循环切换1-2
					else if (setPos.y == 16 || setPos.keyInterrupt == 'a') console.AdjustPointForVirtualConsole(console.rt.x, console.rt.y);
					else if (setPos.y == 19 || setPos.keyInterrupt == 'q') break; // 退出设置

					Sleep(100); // 防止快速切换
				}
			}
			else if (menuPos.y == MENU_START_Y || menuPos.keyInterrupt == 's') started = true; // 开始新游戏
			else if (menuPos.y == MENU_LOAD_Y || menuPos.keyInterrupt == 'r') return false; // 加载存档
			else if (menuPos.y == MENU_EXIT_Y || menuPos.keyInterrupt == 'e')exit(0);
			Sleep(50);
		}
		return true;
	}

	// 游戏主循环
	void gameLoop(const GameConfig& config, bool isNewGame = true) {
		ChessBoard board(config.winMode, isNewGame ? "" : "savegame.dat");
		console.clearScreen();

		bool isSelectingFrom = true;  // 标记输入阶段（选择起始/目标位置）
		Position from, to, transPos;

		while (!board.isGameOver()) {
			board.displayBoard();
			board.getMsgBar().display();

			// 决定输入方式（鼠标/键盘）
			bool useMouse = config.ctrlMode == 1 ||
				(config.ctrlMode == 3 && board.getCurrentPlayer() == Piece::RED) ||
				(config.ctrlMode == 4 && board.getCurrentPlayer() == Piece::BLACK);

			// 获取输入坐标
			if (useMouse) {
				board.getMsgBar().sendMessage("鼠标输入坐标" + to_string(isSelectingFrom ? 1 : 2) + "(x,y)");
				Position pos = GetMousePos();
				transPos.x = pos.x - 1;
				transPos.y = pos.y / 2 - 1;
			}
			else {
				board.getMsgBar().sendMessage("键盘输入坐标" + to_string(isSelectingFrom ? 1 : 2) + "(x,y)");
				console.gotoxy(3, 22);
				console.setColor(ConsoleColor::YELLOW,
					board.getCurrentPlayer() ? ConsoleColor::BLACK : ConsoleColor::RED);
				cout << "第" << (isSelectingFrom ? 1 : 2) << "个:                             ";
				console.gotoxy(9, 22);
				transPos = GetKeyboardPos(board.getMsgBar());
			}

			// 检查坐标有效性
			if (!(transPos.x >= -1 && transPos.x < BOARD_COLS &&
				transPos.y >= -1 && transPos.y < BOARD_ROWS)) {
				board.getMsgBar().addMessage("位置输入错误（超出范围）", true);
				continue;
			}

			// 处理顶部按钮（保存/返回）
			if (transPos.y == -1) {
				if (transPos.x >= -1 && transPos.x <= 2)board.saveGame(); // 保存游戏
				else if (transPos.x >= 7 && transPos.x <= 9) return; // 返回菜单
				else board.getMsgBar().addMessage("无效操作", true);
				continue;
			}

			// 处理棋子选择/移动
			if (isSelectingFrom) {
				from = transPos;
				board.getMsgBar().addMessage("已选择位置: (" + to_string(from.x) + "," + to_string(from.y) + ")");
			}
			else {
				to = transPos;
				board.movePiece(from, to, config.ruleMode == 1);
			}

			isSelectingFrom = !isSelectingFrom; // 切换输入阶段
			Sleep(50);
		}

		// 游戏结束显示结果
		board.displayBoard();
		board.getMsgBar().addMessage("游戏结束！" + string(board.getWinner() == Piece::RED ? "红方获胜！" : "黑方获胜！"));
		board.getMsgBar().display();
		Sleep(3000); // 显示结果3秒
	}
}

int main() {
	ChineseChess::console.initialize();
	ChineseChess::GameConfig config;

	while (true) {
		bool isNewGame = ChineseChess::showMainMenu(config);
		ChineseChess::gameLoop(config, isNewGame);
	}

	return 0;
}
