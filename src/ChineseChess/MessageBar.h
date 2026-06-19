#pragma once
#include <string>
#include <vector>

namespace ChineseChess {

    class MessageBar {
    public:
        MessageBar(int topRow, int maxLines = 3);

        void AddMessage(const std::string& text, bool isError = false);
        void SendMessage(const std::string& text, bool isError = false);
        void Clear();
        void Display() const;

    private:
        int topRow_;
        int maxLines_;
        std::vector<std::string> messages_;
        int counter_ = 0; // 消息编号（替换静态局部变量）
    };

} // namespace ChineseChess