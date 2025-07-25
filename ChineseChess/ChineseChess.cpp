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

// ȫ�ֹ��ߺ������ظ�����ַ���
void repeat(const string& content, int times) {
	for (int i = 0; i < times; ++i) {
		cout << content;
	}
}

namespace ChineseChess {
	// �������壺���̲���
	const int BOARD_ROWS = 10;    // ��������
	const int BOARD_COLS = 9;     // ��������
	const int RED_GENERAL_ROW_MIN = 7;  // ��˧�Ź��з�Χ
	const int RED_GENERAL_ROW_MAX = 9;
	const int RED_GENERAL_COL_MIN = 3;  // ��˧�Ź��з�Χ
	const int RED_GENERAL_COL_MAX = 5;
	const int BLACK_GENERAL_ROW_MIN = 0; // �ڽ��Ź��з�Χ
	const int BLACK_GENERAL_ROW_MAX = 2;
	const int BLACK_GENERAL_COL_MIN = 3; // �ڽ��Ź��з�Χ
	const int BLACK_GENERAL_COL_MAX = 5;
	const int RIVER_ROW = 4;      // ���Ӻ���������

	// ���泣��
	const int MSG_BAR_TOP_ROW = 24;    // ��Ϣ����ʼ��
	const int MSG_BAR_MAX_LINES = 3;   // ��Ϣ���������
	const int MENU_SETTINGS_Y = 10;    // ���˵�"����"��������
	const int MENU_START_Y = 12;       // ���˵�"��ʼ��Ϸ"��������
	const int MENU_LOAD_Y = 14;        // ���˵�"�ָ�����"��������
	const int MENU_EXIT_Y = 16;        // ���˵�"�˳�"��������

	// ��ɫö�٣�����ɫ+ǰ��ɫ��
	enum class ConsoleColor {
		BLACK = 0,
		RED = 4,
		YELLOW = 14
	};

	// ����̨�����ࣨ��װ����̨������
	class Console {
	public:
		bool isVC = false;
		POINT pa = {};
		POINT pb = {};
		POINT pt = {};
		POINT rt = { 37,26 };
	private:
		HWND window = nullptr;                    // ����̨���ھ��
		HANDLE outputHandle = nullptr;            // ������
		CONSOLE_FONT_INFO fontInfo = {};     // ������Ϣ
		CONSOLE_CURSOR_INFO cursorInfo = {}; // �����Ϣ

		void setupConsoleWindow() {
			HWND consoleWindow = GetConsoleWindow();//��ֹ����
			SetWindowLong(consoleWindow, GWL_STYLE,
				GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

			SMALL_RECT windowRect = { 0, 0, rt.x, rt.y };//���ڴ�С
			CONSOLE_SCREEN_BUFFER_INFO bufferInfo;//ȥ��������
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
			mode &= ~ENABLE_QUICK_EDIT_MODE; // ���ÿ��ٱ༭ģʽ
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

			GetConsoleCursorInfo(outputHandle, &cursorInfo);// ���ع��
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
			cout << "���";
			getPos(pa);
			gotoxy(x - 5, y);
			setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
			cout << "���";
			setColor(ConsoleColor::YELLOW, ConsoleColor::RED);
			cout << "@";
			getPos(pb);
		}
		// ��ȡ������Ϣ��������ת��ʹ�ã�
		const CONSOLE_FONT_INFO& getFontInfo() const { return fontInfo; }
		HWND getWindowHandle() const { return window; }
	};

	Console console;

	// ��Ϣ���ࣨ�ײ��̶�����������ʾ��ʾ��Ϣ��
	class MessageBar {
	private:
		vector<string> messages;  // ��Ϣ�洢����
		int maxLines;             // �����ʾ����
		int barTopRow;            // ��Ϣ����ʼ��λ��

	public:
		MessageBar(int topRow, int lines = 3) : barTopRow(topRow), maxLines(lines) { messages.reserve(maxLines); }

		// �����Ϣ��֧�ֲ�ͬ���ͣ���ͨ/����
		void addMessage(const string& text, bool isError = false) {
			static int idx = 0;
			idx = (idx + 1) % 10;
			string msg = to_string(idx) + (isError ? "[����] " : "[��ʾ] ") + text;

			// �����������ʱ�Ƴ������Ϣ
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

		// ��ʾ��Ϣ�����̶��ڵײ���
		void display() const {
			// ������Ϣ���ָ���
			console.gotoxy(0, barTopRow - 1);
			console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
			repeat("��", 39);

			// ��ʾ��Ϣ
			for (int i = 0; i < maxLines; ++i) {
				console.gotoxy(0, barTopRow + i);
				if (i < messages.size()) {
					// ������Ϣ��ʾ��ɫ���֣���ͨ��Ϣ��ɫ
					if (messages[i].find("[����]") != string::npos) {
						console.setColor(ConsoleColor::YELLOW, ConsoleColor::RED);
					}
					else {
						console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
					}
					cout << messages[i];
				}
				// ���հ׸��Ǿ���Ϣ
				repeat(" ", 39 - (i < messages.size() ? (int)messages[i].size() : 0));
			}
			console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
		}
	};

	// λ�ýṹ�壨x��ʾ�У�y��ʾ�У�
	struct Position {
		int x, y; // x:��, y:��
		int keyInterrupt = -1;
		Position(int x = 0, int y = 0) : x(x), y(y) {}
	};

	// ��ȡ�����������λ��
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

				// ����ת���߼����������������ת��Ϊ���̸�������
				auto& fontInfo = console.getFontInfo();
				if (fontInfo.dwFontSize.X == 0 || fontInfo.dwFontSize.Y == 0) {
					if (console.isVC) {
						pos = trans((mousePos.x - console.pa.x) / (double)(console.pb.x - console.pa.x) * (console.pt.x - 1),
							(mousePos.y - console.pa.y) / (double)(console.pb.y - console.pa.y) * (console.pt.y - 1) + 1);
					}
					else {
						cerr << "��֧�����,ǰ�����ø���";
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

	// ��ȡ�������������
	Position GetKeyboardPos(MessageBar& msgBar) {
		Position pos;
		string input;
		getline(cin, input);
		istringstream iss(input);
		if (!(iss >> pos.x >> pos.y)) {
			pos.x = INT32_MAX;
			pos.y = INT32_MAX;
			msgBar.addMessage("�����ʽ����������\"x y\"", true);
		}
		return pos;
	}

	// ���ӻ���
	class Piece {
	public:
		enum Type {
			EMPTY,
			// �췽����
			R_GENERAL, R_ADVISOR, R_ELEPHANT, R_HORSE, R_CHARIOT, R_CANNON, R_SOLDIER,
			// �ڷ�����
			B_GENERAL, B_ADVISOR, B_ELEPHANT, B_HORSE, B_CHARIOT, B_CANNON, B_SOLDIER
		};

		enum Color {
			RED,    // �췽
			BLACK,  // �ڷ�
			NONE    // ����ɫ����λ�ã�
		};

		Type type;    // ��������
		Color color;  // ��ɫ
		bool alive;   // �Ƿ���

		Piece(Type type = EMPTY, Color color = NONE)
			: type(type), color(color), alive(false) {
		}

		virtual ~Piece() = default;

		// ��֤�ƶ��Ƿ���Ч
		virtual bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const = 0;

		// ��ȡ���ӵ������ַ�
		string getChineseChar() const {
			static const string pieceChars[] = {
				"��", // ��
				"˧", "��", "��", "��", "��", "��", "��", // �췽
				"��", "ʿ", "��", "��", "��", "��", "��"  // �ڷ�
			};
			return pieceChars[type];
		}

		Color getColor() const { return color; }

	protected:
		// ͨ�ü�飺���ܳԼ�������
		bool isSameColor(const vector<vector<shared_ptr<Piece>>>& board, Position to) const {
			return board[to.y][to.x] && board[to.y][to.x]->color == color;
		}
	};

	// ���ӹ�����������
	shared_ptr<Piece> PieceFactory(Piece::Type type, Piece::Color color);

	// ����
	class Chariot : public Piece {
	public:
		Chariot(Color color) : Piece(color == RED ? R_CHARIOT : B_CHARIOT, color) {
			alive = true;
		}

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// ���ܳԼ�������
			if (isSameColor(board, to)) return false;

			// ����ֱ���ƶ�
			if (from.x != to.x && from.y != to.y) return false;

			// ���·���Ƿ����ϰ���
			if (from.x == to.x) { // ��ֱ�ƶ�������ͬ��
				int step = (from.y < to.y) ? 1 : -1;
				for (int y = from.y + step; y != to.y; y += step) {
					if (board[y][from.x] != nullptr) return false;
				}
			}
			else { // ˮƽ�ƶ�������ͬ��
				int step = (from.x < to.x) ? 1 : -1;
				for (int x = from.x + step; x != to.x; x += step) {
					if (board[from.y][x] != nullptr) return false;
				}
			}

			return true;
		}
	};

	// ����
	class Horse : public Piece {
	public:
		Horse(Color color) : Piece(color == RED ? R_HORSE : B_HORSE, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// ���ܳԼ�������
			if (isSameColor(board, to)) return false;

			// ����"��"��
			int dx = abs(from.x - to.x);
			int dy = abs(from.y - to.y);
			if (!((dx == 1 && dy == 2) || (dx == 2 && dy == 1))) return false;

			// ��������Ƿ񱻰�
			Position legPos = from;
			if (dx == 2) legPos.x += (to.x > from.x) ? 1 : -1; // ˮƽ�����ƶ�
			else legPos.y += (to.y > from.y) ? 1 : -1; // ��ֱ�����ƶ�

			return board[legPos.y][legPos.x] == nullptr;
		}
	};

	// ��/����
	class Elephant : public Piece {
	public:
		Elephant(Color color) : Piece(color == RED ? R_ELEPHANT : B_ELEPHANT, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// ���ܳԼ�������
			if (isSameColor(board, to)) return false;

			// ����"��"��
			int dx = abs(from.x - to.x), dy = abs(from.y - to.y);
			if (dx != 2 || dy != 2) return false;

			// ��������Ƿ���
			Position eyePos((from.x + to.x) / 2, (from.y + to.y) / 2);
			if (board[eyePos.y][eyePos.x] != nullptr) return false;

			// ���ܹ���
			if ((color == RED && to.y < 5) || (color == BLACK && to.y > 4)) return false;

			return true;
		}
	};

	// ʿ/����
	class Advisor : public Piece {
	public:
		Advisor(Color color) : Piece(color == RED ? R_ADVISOR : B_ADVISOR, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// ���ܳԼ�������
			if (isSameColor(board, to)) return false;

			// ʿ��б��һ��
			int dx = abs(from.x - to.x), dy = abs(from.y - to.y);
			if (dx != 1 || dy != 1) return false;

			// ���ܳ��Ź�
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

	// ��/˧��
	class General : public Piece {
	public:
		General(Color color) : Piece(color == RED ? R_GENERAL : B_GENERAL, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// ���ܳԼ�������
			if (isSameColor(board, to)) return false;

			// �����������˧���棨�ɽ���
			if (board[to.y][to.x] &&
				(board[to.y][to.x]->type == Piece::B_GENERAL ||
					board[to.y][to.x]->type == Piece::R_GENERAL) &&
				to.x == from.x) {

				int minY = min(to.y, from.y);
				int maxY = max(to.y, from.y);
				for (int y = minY + 1; y < maxY; ++y) {
					if (board[y][to.x] != nullptr) {
						return false; // �м��������赲
					}
				}
				return true;
			}

			// �����ƶ���ֻ����һ��ֱ��
			int dx = abs(from.x - to.x), dy = abs(from.y - to.y);
			if ((dx + dy) != 1) return false;

			// ���ܳ��Ź�
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

	// ����
	class Cannon : public Piece {
	public:
		Cannon(Color color) : Piece(color == RED ? R_CANNON : B_CANNON, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// ���ܳԼ�������
			if (isSameColor(board, to)) return false;

			// ����ֱ���ƶ�
			if (from.x != to.x && from.y != to.y) return false;

			// ����·���ϵ��ϰ�������
			int obstacleCount = 0;
			if (from.x == to.x) { // ��ֱ�ƶ�
				int step = (from.y < to.y) ? 1 : -1;
				for (int y = from.y + step; y != to.y; y += step) {
					if (board[y][from.x] != nullptr) obstacleCount++;
				}
			}
			else { // ˮƽ�ƶ�
				int step = (from.x < to.x) ? 1 : -1;
				for (int x = from.x + step; x != to.x; x += step) {
					if (board[from.y][x] != nullptr) obstacleCount++;
				}
			}

			// �ڷ�ɽ���򣺳�����һ���ϰ���ƶ���0��
			bool isCapturing = (board[to.y][to.x] != nullptr);
			return (isCapturing && obstacleCount == 1) || (!isCapturing && obstacleCount == 0);
		}
	};

	// ʿ��/����
	class Soldier : public Piece {
	public:
		Soldier(Color color) : Piece(color == RED ? R_SOLDIER : B_SOLDIER, color) { alive = true; }

		bool isValidMove(const vector<vector<shared_ptr<Piece>>>& board,
			Position from, Position to) const override {
			// ���ܳԼ�������
			if (isSameColor(board, to)) return false;

			// �ƶ���Χ��飺ֻ����һ��
			int dx = abs(from.x - to.x);
			int dy = to.y - from.y;
			if (dx > 1 || abs(dy) > 1 || (dx == 1 && dy != 0)) return false;

			// ǰ�������飨������ϣ��ڱ����£�
			if ((color == RED && dy > 0) || (color == BLACK && dy < 0)) return false;

			// δ���Ӳ��ܺ����ƶ�
			if (dx == 1 && ((color == RED && from.y > 4) || (color == BLACK && from.y < 5))) return false;

			return true;
		}
	};

	// ���ӹ���ʵ��
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

	// �浵������
	class SaveManager {
	public:
		// ������Ϸ״̬���ļ�
		static bool saveGame(const vector<vector<shared_ptr<Piece>>>& board,
			Piece::Color currentPlayer, const string& filename) {
			ofstream file(filename);
			if (!file.is_open()) return false;

			// ���浱ǰ���
			file << (currentPlayer == Piece::RED ? "RED" : "BLACK") << endl;

			// ��������״̬
			for (int y = 0; y < BOARD_ROWS; ++y) {
				for (int x = 0; x < BOARD_COLS; ++x) {
					file << (board[y][x] ? static_cast<int>(board[y][x]->type) : 0) << " ";
				}
				file << endl;
			}
			return true;
		}

		// ���ļ�������Ϸ״̬
		static bool loadGame(vector<vector<shared_ptr<Piece>>>& board,
			Piece::Color& currentPlayer, const string& filename) {
			ifstream file(filename);
			if (!file.is_open()) return false;

			// ��ʼ������
			board = vector<vector<shared_ptr<Piece>>>(BOARD_ROWS,
				vector<shared_ptr<Piece>>(BOARD_COLS, nullptr));

			// ��ȡ��ǰ���
			string playerStr;
			file >> playerStr;
			currentPlayer = (playerStr == "RED") ? Piece::RED : Piece::BLACK;

			// ��ȡ����״̬
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

	// ������
	class ChessBoard {
	private:
		vector<vector<shared_ptr<Piece>>> board; // 10��9�����̣�board[y][x]��
		Piece::Color currentPlayer;
		bool gameOver;
		Piece::Color winner;
		int winMode;
		MessageBar msgBar;  // ��Ϣ����Ա

		// ����һ�����ӵ�ͨ�ú����������ظ����룩
		void placePieces(Piece::Color color, int backRow, int cannonRow, int soldierRow) {
			// ���ó�������ʿ����
			board[backRow][0] = make_shared<Chariot>(color);
			board[backRow][1] = make_shared<Horse>(color);
			board[backRow][2] = make_shared<Elephant>(color);
			board[backRow][3] = make_shared<Advisor>(color);
			board[backRow][4] = make_shared<General>(color);
			board[backRow][5] = make_shared<Advisor>(color);
			board[backRow][6] = make_shared<Elephant>(color);
			board[backRow][7] = make_shared<Horse>(color);
			board[backRow][8] = make_shared<Chariot>(color);

			// ������
			board[cannonRow][1] = make_shared<Cannon>(color);
			board[cannonRow][7] = make_shared<Cannon>(color);

			// ���ñ�/��
			board[soldierRow][0] = make_shared<Soldier>(color);
			board[soldierRow][2] = make_shared<Soldier>(color);
			board[soldierRow][4] = make_shared<Soldier>(color);
			board[soldierRow][6] = make_shared<Soldier>(color);
			board[soldierRow][8] = make_shared<Soldier>(color);
		}

		// �����Ϸ�Ƿ��������/˧�Ƿ񱻳ԣ�
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
			msgBar.addMessage("��Ϸ��ʼ���췽���У��������ѡ���ƶ�");
		}

		// �Ӵ浵����
		ChessBoard(int winMode, const string& filename) : board(BOARD_ROWS, vector<shared_ptr<Piece>>(BOARD_COLS, nullptr)),
			winMode(winMode), msgBar(MSG_BAR_TOP_ROW, MSG_BAR_MAX_LINES) {
			if (SaveManager::loadGame(board, currentPlayer, filename)) {
				gameOver = false;
				winner = Piece::NONE;
				msgBar.addMessage("(�Ѽ��ش浵)��ǰ�غ�: " + string(currentPlayer == Piece::RED ? "�췽" : "�ڷ�"));
			}
			else {
				initializeBoard();
				currentPlayer = Piece::RED;
				gameOver = false;
				winner = Piece::NONE;
				msgBar.addMessage("(�޴浵)����Ϸ:�췽����");
			}
		}

		// ��ȡ��Ϣ�������ⲿ�����Ϣ��
		MessageBar& getMsgBar() { return msgBar; }

		// ��ʼ������
		void initializeBoard() {
			placePieces(Piece::BLACK, 0, 2, 3); // �ڷ����ӣ�����0����2����3��
			placePieces(Piece::RED, 9, 7, 6);   // �췽���ӣ�����9����7����6��
		}

		// ��ʾ����
		void displayBoard() const {
			//console.clearScreen();
			console.gotoxy(0, 0);
			console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
			cout << "����                             ����\n";
			cout << "   0   1   2   3   4   5   6   7   8\n";

			for (int y = 0; y < BOARD_ROWS; ++y) {
				console.gotoxy(0, 2 * y + 2);
				console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
				cout << " " << y;

				for (int x = 0; x < BOARD_COLS; ++x) {
					cout << " ";
					auto piece = board[y][x];
					if (piece) {
						// ����������ɫ���췽(�Ƶ׺�) �ڷ�(�Ƶ׺�)
						console.setColor(ConsoleColor::YELLOW,
							piece->color == Piece::RED ? ConsoleColor::RED : ConsoleColor::BLACK);
						cout << piece->getChineseChar();
						console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
					}
					else {
						// �������̸���
						drawGrid(y, x);
					}
					if (x != 8) cout << "��";
				}

				cout << " \n";
				console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);

				// ���ƾŹ���б�ߺͳ��Ӻ���
				drawSpecialLines(y + 2);
			}

			// ��ʾ��ǰ�غ����
			console.gotoxy(0, MSG_BAR_TOP_ROW - 2);
			console.setColor(ConsoleColor::YELLOW,
				currentPlayer ? ConsoleColor::BLACK : ConsoleColor::RED);
			cout << (currentPlayer ? "��" : "��") << endl;

			// ��ʾ��Ϣ��
			msgBar.display();
		}

		// ������������
		void drawGrid(int y, int x) const {
			if (y == 0 && x == 0) cout << "����";
			else if (y == 0 && x == 8) cout << "��";
			else if (y == 9 && x == 0) cout << "����";
			else if (y == 9 && x == 8) cout << "��";
			else if (x == 0) cout << "����";
			else if (x == 8) cout << "��";
			else if ((y == 9 || y == 4) && x < 8) cout << "�ة�";
			else if ((y == 0 || y == 5) && x < 8) cout << "�Щ�";
			else cout << "�੤";
		}

		// ���������������Ź���б�ߺͳ��Ӻ��磩
		void drawSpecialLines(int row) const {
			switch (row) {
			case 2: // �ڷ��Ź�б��
				if (winMode == 1) cout << "  ��  ��  ��  ���v���u��  ��  ��  �� \n";
				else cout << "   ��   ��   ��   �� �v �� �u ��   ��   ��   �� \n";
				break;
			case 3: // �ڷ��Ź�б��
				if (winMode == 1) cout << "  ��  ��  ��  ���u���v��  ��  ��  �� \n";
				else cout << "   ��   ��   ��   �� �u �� �v ��   ��   ��   �� \n";
				break;
			case 6: // ���Ӻ���
				if (winMode == 1) cout << "  ��       ����       ����        �� \n";
				else cout << "   ��        ����       ����        �� \n";
				break;
			case 9: // �췽�Ź�б��
				if (winMode == 1) cout << "  ��  ��  ��  ���v���u��  ��  ��  �� \n";
				else cout << "   ��   ��   ��   �� �v �� �u ��   ��   ��   �� \n";
				break;
			case 10: // �췽�Ź�б��
				if (winMode == 1) cout << "  ��  ��  ��  ���u���v��  ��  ��  �� \n";
				else cout << "   ��   ��   ��   �� �u �� �v ��   ��   ��   �� \n";
				break;
			default: // ��ͨ����
				if (row >= 11)return;
				if (winMode == 1) cout << "  ��  ��  ��  ��  ��  ��  ��  ��  �� \n";
				else cout << "   ��   ��   ��   ��   ��   ��   ��   ��   �� \n";
				break;
			}
		}

		// �ƶ�����
		bool movePiece(Position from, Position to, bool enforceRules = true) {
			// �����ʼλ���Ƿ�������
			if (!board[from.y][from.x]) {
				msgBar.addMessage("��ʼλ��û�����ӣ�", true);
				return false;
			}

			// ����Ƿ�ѡ�񼺷�����
			if (board[from.y][from.x]->color != currentPlayer) {
				msgBar.addMessage("��ѡ��" + string(currentPlayer == Piece::RED ? "��" : "��") + "�������ӣ�", true);
				return false;
			}

			// ����ƶ��Ƿ���Ϲ���
			if (enforceRules && !board[from.y][from.x]->isValidMove(board, from, to)) {
				msgBar.addMessage("���ƶ����������ӹ���", true);
				return false;
			}

			// �ƶ��ɹ���ʾ
			string pieceName = board[from.y][from.x]->getChineseChar();
			msgBar.addMessage("���ƶ�" + pieceName + "��(" + to_string(from.x) + "," + to_string(from.y) +
				") �� (" + to_string(to.x) + "," + to_string(to.y) + ")");

			// ִ���ƶ�
			board[to.y][to.x] = board[from.y][from.x];
			board[from.y][from.x] = nullptr;

			checkGameOver(); // �����Ϸ�Ƿ����
			// �л����
			currentPlayer = (currentPlayer == Piece::RED) ? Piece::BLACK : Piece::RED;
			return true;
		}

		Piece::Color getCurrentPlayer() const { return currentPlayer; }

		// ������Ϸ
		bool saveGame(const string& filename = "savegame.dat") {
			bool result = SaveManager::saveGame(board, currentPlayer, filename);
			if (result) msgBar.addMessage("��Ϸ�ѱ����� " + filename);
			else msgBar.addMessage("����ʧ�ܣ�", true);
			return result;
		}

		bool isGameOver() const { return gameOver; }
		Piece::Color getWinner() const { return winner; }
	};

	// ��Ϸ����
	struct GameConfig {
		int ctrlMode = 1;   // ����ģʽ��1:��� 2:���� 3:��� 4:����
		int ruleMode = 1;   // ����ģʽ��1:���� 2:�رգ�
		int winMode = 2;    // ����ģʽ��1:��7 2:��10��
	};

	// ��ʾ���˵�
	bool showMainMenu(GameConfig& config) {
		bool started = false;
		while (!started) {
			console.clearScreen();
			console.gotoxy(0, 0);

			console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
			cout << "��----------------------------------" << (console.isVC ? "--" : "") << "��\n";
			repeat("|                                    |\n", 5);
			cout << "|              �й�����              |\n";
			repeat("|                                    |\n", 3);
			cout << "|           >    ����(C)             |\n";
			repeat("|                                    |\n", 1);
			cout << "|           >  ��ʼ��Ϸ(S)           |\n";
			repeat("|                                    |\n", 1);
			cout << "|           >  �ָ�����(R)           |\n";
			repeat("|                                    |\n", 1);
			cout << "|           >    �˳�(E)             |\n";
			repeat("|                                    |\n", 3);
			cout << "|          �벻Ҫ�����߿�λ��        |\n";
			cout << "|   ��Ϊ�����նˣ���ɫ��ΪCampbell   |\n";
			repeat("|                                    |\n", 1);
			cout << "��----------------------------------" << (console.isVC ? "--" : "") << "��\n";

			Position menuPos = GetMousePos({ 'c','s','r','e' });
			if (menuPos.y == MENU_SETTINGS_Y || menuPos.keyInterrupt == 'c') {
				// ��ʾ���ò˵�
				while (true) {
					console.clearScreen();
					console.gotoxy(0, 0);

					console.setColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
					cout << "��----------------------------------" << (console.isVC ? "--" : "") << "��\n";
					repeat("|                                    |\n", 5);
					cout << "|                ����                |\n";
					repeat("|                                    |\n", 3);
					cout << "|           >    ����(C)  ";
					switch (config.ctrlMode) {
					case 1: cout << "���       |\n"; break;
					case 2: cout << "����       |\n"; break;
					case 3: cout << "���       |\n"; break;
					default: cout << "����       |\n"; break;
					}
					repeat("|                                    |\n", 1);
					cout << "|           >    ����(R)  ";
					cout << (config.ruleMode == 1 ? "����       |\n" : "�ر�       |\n");
					repeat("|                                    |\n", 1);
					cout << "|           >    ϵͳ(S)   ";
					cout << (config.winMode == 1 ? " 7        |\n" : " 10       |\n");
					repeat("|                                    |\n", 1);
					cout << "|           >    �����ն�(A)         |\n";
					cout << "|                ����У׼            |\n";
					repeat("|                                    |\n", 1);
					cout << "|           >    �˳�(Q)             |\n";
					repeat("|                                    |\n", 2);
					cout << "|          �벻Ҫ�����߿�λ��        |\n";
					cout << "��----------------------------------" << (console.isVC ? "--" : "") << "��\n";

					Position setPos = GetMousePos({ 'c','r','s','q' ,'a' });
					if (setPos.y == 10 || setPos.keyInterrupt == 'c') config.ctrlMode = (config.ctrlMode % 4) + 1; // ѭ���л�1-4
					else if (setPos.y == 12 || setPos.keyInterrupt == 'r') config.ruleMode = (config.ruleMode % 2) + 1; // ѭ���л�1-2
					else if (setPos.y == 14 || setPos.keyInterrupt == 's') config.winMode = (config.winMode % 2) + 1;   // ѭ���л�1-2
					else if (setPos.y == 16 || setPos.keyInterrupt == 'a') console.AdjustPointForVirtualConsole(console.rt.x, console.rt.y);
					else if (setPos.y == 19 || setPos.keyInterrupt == 'q') break; // �˳�����

					Sleep(100); // ��ֹ�����л�
				}
			}
			else if (menuPos.y == MENU_START_Y || menuPos.keyInterrupt == 's') started = true; // ��ʼ����Ϸ
			else if (menuPos.y == MENU_LOAD_Y || menuPos.keyInterrupt == 'r') return false; // ���ش浵
			else if (menuPos.y == MENU_EXIT_Y || menuPos.keyInterrupt == 'e')exit(0);
			Sleep(50);
		}
		return true;
	}

	// ��Ϸ��ѭ��
	void gameLoop(const GameConfig& config, bool isNewGame = true) {
		ChessBoard board(config.winMode, isNewGame ? "" : "savegame.dat");
		console.clearScreen();

		bool isSelectingFrom = true;  // �������׶Σ�ѡ����ʼ/Ŀ��λ�ã�
		Position from, to, transPos;

		while (!board.isGameOver()) {
			board.displayBoard();
			board.getMsgBar().display();

			// �������뷽ʽ�����/���̣�
			bool useMouse = config.ctrlMode == 1 ||
				(config.ctrlMode == 3 && board.getCurrentPlayer() == Piece::RED) ||
				(config.ctrlMode == 4 && board.getCurrentPlayer() == Piece::BLACK);

			// ��ȡ��������
			if (useMouse) {
				board.getMsgBar().sendMessage("�����������" + to_string(isSelectingFrom ? 1 : 2) + "(x,y)");
				Position pos = GetMousePos();
				transPos.x = pos.x - 1;
				transPos.y = pos.y / 2 - 1;
			}
			else {
				board.getMsgBar().sendMessage("������������" + to_string(isSelectingFrom ? 1 : 2) + "(x,y)");
				console.gotoxy(3, 22);
				console.setColor(ConsoleColor::YELLOW,
					board.getCurrentPlayer() ? ConsoleColor::BLACK : ConsoleColor::RED);
				cout << "��" << (isSelectingFrom ? 1 : 2) << "��:                             ";
				console.gotoxy(9, 22);
				transPos = GetKeyboardPos(board.getMsgBar());
			}

			// ���������Ч��
			if (!(transPos.x >= -1 && transPos.x < BOARD_COLS &&
				transPos.y >= -1 && transPos.y < BOARD_ROWS)) {
				board.getMsgBar().addMessage("λ��������󣨳�����Χ��", true);
				continue;
			}

			// ��������ť������/���أ�
			if (transPos.y == -1) {
				if (transPos.x >= -1 && transPos.x <= 2)board.saveGame(); // ������Ϸ
				else if (transPos.x >= 7 && transPos.x <= 9) return; // ���ز˵�
				else board.getMsgBar().addMessage("��Ч����", true);
				continue;
			}

			// ��������ѡ��/�ƶ�
			if (isSelectingFrom) {
				from = transPos;
				board.getMsgBar().addMessage("��ѡ��λ��: (" + to_string(from.x) + "," + to_string(from.y) + ")");
			}
			else {
				to = transPos;
				board.movePiece(from, to, config.ruleMode == 1);
			}

			isSelectingFrom = !isSelectingFrom; // �л�����׶�
			Sleep(50);
		}

		// ��Ϸ������ʾ���
		board.displayBoard();
		board.getMsgBar().addMessage("��Ϸ������" + string(board.getWinner() == Piece::RED ? "�췽��ʤ��" : "�ڷ���ʤ��"));
		board.getMsgBar().display();
		Sleep(3000); // ��ʾ���3��
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
