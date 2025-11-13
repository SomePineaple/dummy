#include "game/game.h"

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <boost/process.hpp>
#include <iostream>

using json = nlohmann::json;

void jsonTesting();

int main() {
    jsonTesting();


    return 0;
}

int runTerminalGame() {
    auto game = game::Game{};

    while (!game.isGameOver())
        game.runRound();

    return 0;
}

void jsonTesting() {
    json j = {
        {"name", "Nathan"},
        {"Age", 19},
        {"Date of birth", "08/11/06"},
    };

    std::cout << j << std::endl;

    std::cin >> j;

    std::cout << j << std::endl;

    boost::asio::io_context ctx;
    boost::process::popen p(ctx, "/usr/bin/echo", {"What the fuck is a man doing here"});
    std::string line;
    boost::asio::read_until(p, boost::asio::dynamic_buffer(line), '\n');

    std::cout << "Now we read from the echo: " << line << std::endl;
    p.wait();
}
