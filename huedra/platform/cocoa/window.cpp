#include "window.hpp"

namespace huedra {

bool WindowCocoa::init(const std::string& title, const WindowInput& input) { return true; }

void WindowCocoa::cleanup() {}

bool WindowCocoa::update() { return true; }

void WindowCocoa::setTitle(const std::string& title) {}

void WindowCocoa::setResolution(u32 width, u32 height) {}

void WindowCocoa::setPos(i32 x, i32 y) {}

} // namespace huedra