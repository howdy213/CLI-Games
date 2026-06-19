#include "MessageBar.h"
#include "Console.h"
#include "ChineseChess.h"
#include <format>
#include <iostream>
#undef SendMessage
namespace ChineseChess {

    MessageBar::MessageBar(int topRow, int maxLines)
        : topRow_(topRow), maxLines_(maxLines) {
        messages_.reserve(maxLines_);
    }

    void MessageBar::AddMessage(const std::string& text, bool isError) {
        counter_ = (counter_ + 1) % 100;
        std::string prefix = isError ? "[´íÎó] " : "[ÌáÊ¾] ";
        std::string msg = std::format("{}{}{}", counter_, prefix, text);

        if (messages_.size() >= maxLines_) {
            messages_.erase(messages_.begin());
        }
        messages_.push_back(msg);
    }

    void MessageBar::SendMessage(const std::string& text, bool isError) {
        AddMessage(text, isError);
        Display();
    }

    void MessageBar::Clear() {
        messages_.clear();
    }

    void MessageBar::Display() const {
        auto& console = Console::Instance();
        console.Gotoxy(0, topRow_ - 1);
        console.SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
        Repeat("©¤", 39);

        for (int i = 0; i < maxLines_; ++i) {
            console.Gotoxy(0, topRow_ + i);
            if (i < static_cast<int>(messages_.size())) {
                bool isError = messages_[i].find("[´íÎó]") != std::string::npos;
                console.SetColor(ConsoleColor::YELLOW, isError ? ConsoleColor::RED : ConsoleColor::BLACK);
                std::cout << messages_[i];
                // Çå³ýÐÐÎ²²ÐÁô
                Repeat(" ", 39 - static_cast<int>(messages_[i].size()));
            }
            else {
                console.SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
                Repeat(" ", 39);
            }
        }
        console.SetColor(ConsoleColor::YELLOW, ConsoleColor::BLACK);
    }

} // namespace ChineseChess