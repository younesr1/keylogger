#pragma once
#include <sys/types.h>
#include <numeric>
#include <algorithm>
#include <cassert>
#include <linux/input.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <array>
#include <thread>
#include <unordered_map>
#include <optional>
#include <ranges>

class KeyboardFile {
    private:
    static constexpr auto BUFFER_SIZE = 1024;
    using Buffer = std::array<input_event,BUFFER_SIZE>;
    Buffer buffer_;
    int fd_;

    public:
    constexpr KeyboardFile() noexcept = delete;
    ~KeyboardFile() noexcept {
        auto _ = close(fd_);
        (void)_;
    }
    constexpr KeyboardFile(const KeyboardFile&) noexcept = delete;
    constexpr KeyboardFile &operator=(const KeyboardFile&) const noexcept = delete;
    constexpr KeyboardFile(KeyboardFile &&other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
    KeyboardFile &operator=(KeyboardFile &&other) noexcept {
        fd_ = other.fd_;
        other.fd_ = -1;
        return *this;
    }

    auto read() {
        const auto bytes = ::read(fd_,buffer_.data(),buffer_.size());
        using rng = std::ranges::take_view<std::ranges::ref_view<Buffer>>;
        using ret = std::optional<rng>;
        if (bytes == -1) {
            return ret{};
        }
        assert((bytes%sizeof(input_event)) == 0);
        const auto num_events_read = bytes/sizeof(input_event);
        return ret{buffer_ | std::ranges::views::take(num_events_read)};
    }

    template <typename T>
    requires std::is_convertible_v<T, std::string_view>
    static constexpr auto make(T&& filepath) {
        const auto view = std::string_view(std::forward<T>(filepath));
        const auto fd = open(view.data(),O_RDONLY);
        return fd == -1 ? std::nullopt : std::optional{KeyboardFile(fd)};
    }
    private:
    constexpr KeyboardFile(const int fd) : buffer_{0}, fd_{fd} {
        assert(fd_ != -1);
    }
};

// useful: https://stackoverflow.com/questions/16695432/input-event-structure-description-from-linux-input-h
// from https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/input-event-codes.h
// type is the type of event: key press event is EV_KEY==0x01
// code is the character of that was pressed ie. a==30
// value represents one of button down, button up, and button continuously pressed
class Parser {
    private:
    // TODO: make map constexpr using https://github.com/serge-sans-paille/frozen
    static const std::unordered_map<int64_t,std::string> char_map;
    public:
    constexpr Parser() noexcept = default;
    constexpr ~Parser() noexcept = default;
    constexpr Parser(const Parser&) noexcept = default;
    constexpr Parser& operator=(const Parser&) noexcept = default;
    constexpr Parser(Parser&&) noexcept = default;
    constexpr Parser& operator=(Parser&&) noexcept = default;

    template <typename T>
    requires std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, input_event>
    constexpr std::string parse(T&& data) const {
        constexpr auto is_keyboard_event = [](const auto &event) constexpr {
            return event.type == EV_KEY;
        };
        constexpr auto is_pressed_down = [](const auto &event) constexpr {
            return event.value != 0; // 0 means released
        };
        constexpr auto func = [](const auto &event) constexpr {
            return char_map.contains(event.code) ? char_map.at(event.code) : "?";
        };
        auto transformed = std::forward<T>(data) | std::views::filter(is_keyboard_event) |std::views::filter(is_pressed_down) | std::ranges::views::transform(func);
        return std::accumulate(transformed.begin(),transformed.end(),std::string());
    }
};

const std::unordered_map<int64_t,std::string> Parser::char_map = { 
        {KEY_1, "1"},
        {KEY_2, "2"},
        {KEY_3, "3"},
        {KEY_4, "4"},
        {KEY_5, "5"},
        {KEY_6, "6"},
        {KEY_7, "7"},
        {KEY_8, "8"},
        {KEY_9, "9"},
        {KEY_0, "0"},
        {KEY_Q, "q"},
        {KEY_W, "w"},
        {KEY_E, "e"},
        {KEY_R, "r"},
        {KEY_T, "t"},
        {KEY_Y, "y"},
        {KEY_U, "u"},
        {KEY_I, "i"},
        {KEY_O, "o"},
        {KEY_P, "p"},
        {KEY_A, "a"},
        {KEY_S, "s"},
        {KEY_D, "d"},
        {KEY_F, "f"},
        {KEY_G, "g"},
        {KEY_H, "h"},
        {KEY_J, "j"},
        {KEY_K, "k"},
        {KEY_L, "l"},
        {KEY_Z, "z"},
        {KEY_X, "x"},
        {KEY_C, "c"},
        {KEY_V, "v"},
        {KEY_B, "b"},
        {KEY_N, "n"},
        {KEY_M, "m"},
        {KEY_COMMA, ","},
        {KEY_DOT, "."},
        {KEY_SLASH, "/"},
        {KEY_SEMICOLON, ";"},
        {KEY_APOSTROPHE, "\'"},
        {KEY_LEFTBRACE, "["},
        {KEY_RIGHTBRACE, "]"},
        {KEY_KP7,"7"},
        {KEY_KP8, "8"},
        {KEY_KP9, "9"},
        {KEY_KPMINUS, "-"},
        {KEY_KP4, "4"},
        {KEY_KP5, "5"},
        {KEY_KP6, "6"},
        {KEY_KPPLUS, "+"},
        {KEY_KP1, "1"},
        {KEY_KP2, "2"},
        {KEY_KP3, "3"},
        {KEY_KP0, "0"},
        {KEY_KPDOT, "."},
        {KEY_LEFTBRACE, "{"},
        {KEY_RIGHTBRACE, "}"},
        {KEY_ENTER, "\n"},
        {KEY_BACKSLASH, "\\"},
        {KEY_SPACE, " "},
        {KEY_MINUS, "-"},
        {KEY_EQUAL, "="},
        {KEY_TAB, "\t"}};
