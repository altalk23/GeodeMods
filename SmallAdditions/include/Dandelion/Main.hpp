#pragma once

#include <functional>

namespace tulip::dandelion {
	void registerEnable(std::function<void()> func);
	void registerDisable(std::function<void()> func);
}