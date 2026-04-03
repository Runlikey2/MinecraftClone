#include "core/Engine.h"
#include <cstdio>
#include <stdexcept>

int main() {
    try {
        mc::Engine engine;
        engine.run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Fatal error: %s\n", e.what());
        return 1;
    }
    return 0;
}
