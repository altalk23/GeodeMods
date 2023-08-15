
#include <Geode/modify/AppDelegate.hpp>

using namespace geode::prelude;

static bool s_didEnterBackground = false;

struct DisableSaveUnfocus : Modify<DisableSaveUnfocus, AppDelegate> {
	static void onModify(auto& self) {
		(void)self.setHookPriority("AppDelegate::applicationDidEnterBackground", 0x10000);
		(void)self.setHookPriority("AppDelegate::trySaveGame", 0x10000);
	}

	virtual void applicationDidEnterBackground() {
		s_didEnterBackground = true;
		AppDelegate::applicationDidEnterBackground();
		s_didEnterBackground = false;
	}

	virtual void trySaveGame() {
		if (!Mod::get()->getSettingValue<bool>("disable-save-unfocus") || !s_didEnterBackground) {
			AppDelegate::trySaveGame();
		}
	}
};