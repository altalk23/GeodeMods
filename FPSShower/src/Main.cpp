#include <FPSOverlay.hpp>
#include <FPSUpdate.hpp>
#include <Geode/Loader.hpp>
#include <Geode/loader/SettingEvent.hpp>

using namespace geode::prelude;
using namespace tulip::fps;

#include <Geode/modify/LoadingLayer.hpp>

struct LoadingLayerCheck : Modify<LoadingLayerCheck, LoadingLayer> {
	bool init(bool fromReload) {
		if (!LoadingLayer::init(fromReload)) {
			return false;
		}
		FPSUpdate::get()->releaseOverlay();
		return true;
	}
};

static bool s_loaded = false;

#include <Geode/modify/MenuLayer.hpp>

struct MenuLayerCheck : Modify<MenuLayerCheck, MenuLayer> {
	bool init() {
		if (!MenuLayer::init()) {
			return false;
		}
		s_loaded = true;
		FPSUpdate::get()->setOverlay(FPSOverlay::create());
		return true;
	}
};

$on_mod(Enabled) {
	log::info("Enabled");
	CCDirector::get()->getScheduler()->scheduleUpdateForTarget(FPSUpdate::get(), 0, false);

	if (s_loaded) {
		FPSUpdate::get()->setOverlay(FPSOverlay::create());
	}
}

$on_mod(Disabled) {
	log::info("Disabled");
	CCDirector::get()->getScheduler()->unscheduleUpdateForTarget(FPSUpdate::get());

	if (s_loaded) {
		FPSUpdate::get()->releaseOverlay();
	}
}

static auto colorChanges = listenForSettingChanges(
	"text-color",
	+[](ccColor3B value) {
		FPSUpdate::get()->setOverlay(FPSOverlay::create());
	}
);