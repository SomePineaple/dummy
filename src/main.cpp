#include <string>

int main(int numArgs, const char** args) {
    if (numArgs != 2)
        return 1;

    std::string playerNumber = args[0];
    std::string botNumber = args[1];

    return 0;
}
