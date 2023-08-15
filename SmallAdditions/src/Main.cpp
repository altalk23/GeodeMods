#include <Geode/Geode.hpp>
#include <Main.hpp>
#include <vector>

using namespace geode::prelude;
using namespace tulip::dandelion;

static inline std::vector<std::function<void()>> s_enables;
static inline std::vector<std::function<void()>> s_disables;

void tulip::dandelion::registerEnable(std::function<void()> func) {
	s_enables.push_back(func);
}

void tulip::dandelion::registerDisable(std::function<void()> func) {
	s_disables.push_back(func);
}

$execute {
	auto res = Mod::get()->patch(reinterpret_cast<void*>(base::get() + 0x3aa9d8), { 0x00 });
}

$on_mod(Enabled) {
	log::info("Enabled");
	for (auto& func : s_enables) {
		func();
	}
}

$on_mod(Disabled) {
	log::info("Disabled");
	for (auto& func : s_disables) {
		func();
	}
}
