#import <Cocoa/Cocoa.h>
#include <Geode/Bindings.hpp>
#include <Geode/DefaultInclude.hpp>
#include <Geode/Loader.hpp>
#import <Geode/cocos/platform/mac/EAGLView.h>
#include <Geode/utils/ObjcHook.hpp>
#import <objc/runtime.h>

using namespace geode::prelude;

// TODO:
namespace tulip::fps {
	template <class Lambda>
	inline void openGLWrapped(Lambda&& func) {
		if (!geode::Loader::get()->isModLoaded("alk.fps-bypass")) {
			return func();
		}

		NSOpenGLView* openGLView = [NSClassFromString(@"EAGLView") sharedEGLView];
		NSOpenGLContext* glContext = [openGLView openGLContext];

		CGLLockContext([glContext CGLContextObj]);

		func();

		CGLUnlockContext([glContext CGLContextObj]);
	}
}

using namespace tulip::fps;

#include <Geode/modify/PlatformToolbox.hpp>

struct ToggleFullscreenReplace : Modify<ToggleFullscreenReplace, PlatformToolbox> {
	static void toggleFullScreen(bool fullscreen) {
		auto window = [[NSApplication sharedApplication] mainWindow];

		[window toggleFullScreen:nil];
	}
};

static IMP s_applicationDidFinishLaunching;

void applicationDidFinishLaunching(id self, SEL selector, NSNotification* notification) {
	auto windowed = GameManager::get()->getGameVariable("0025");
	if (!windowed) {
		GameManager::get()->setGameVariable("0025", true);
	}
	// using Type = decltype(&applicationDidFinishLaunching);
	// reinterpret_cast<Type>(s_applicationDidFinishLaunching)(self, selector, notification);

	[self applicationDidFinishLaunching:notification];

	auto window = *(NSWindow**)((uintptr_t)self + 0x8);

	[window setBackgroundColor:NSColor.whiteColor];
	[window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

	if (!windowed) {
		PlatformToolbox::toggleFullScreen(true);
	}
}

void windowWillEnterFullScreen(id self, SEL selector, NSNotification* notification) {
	auto screenFrame = [[NSScreen mainScreen] frame];

	CCEGLView::sharedOpenGLView()->setFrameSize(screenFrame.size.width, screenFrame.size.height);
	CCDirector::get()->updateScreenScale({ (float)screenFrame.size.width,
										   (float)screenFrame.size.height });

	[[NSClassFromString(@"EAGLView") sharedEGLView] setNeedsDisplay:YES];

	GameManager::get()->setGameVariable("0025", false);
}

void windowWillExitFullScreen(id self, SEL selector, NSNotification* notification) {
	auto screenFrame = [[NSScreen mainScreen] frame];

	CCEGLView::sharedOpenGLView()->setFrameSize(screenFrame.size.width, screenFrame.size.height);
	CCDirector::get()->updateScreenScale({ (float)screenFrame.size.width,
										   (float)screenFrame.size.height });

	[[NSClassFromString(@"EAGLView") sharedEGLView] setNeedsDisplay:YES];

	GameManager::get()->setGameVariable("0025", true);
}

void applicationWillBecomeActive(id self, SEL selector, NSNotification* notification) {
	openGLWrapped([]() {
		auto windowed = GameManager::get()->getGameVariable("0025");
		if (windowed) {
			AppDelegate::get()->applicationWillEnterForeground();
		}
		else {
			AppDelegate::get()->applicationWillBecomeActive();
		}
	});
}

void applicationWillResignActive(id self, SEL selector, NSNotification* notification) {
	openGLWrapped([]() {
		auto windowed = GameManager::get()->getGameVariable("0025");
		if (windowed) {
			AppDelegate::get()->applicationDidEnterBackground();
		}
		else {
			AppDelegate::get()->applicationWillResignActive();
		}
	});
}

template <class Type>
IMP replaceMethod(Class class_, SEL selector, Type function) {
	return class_replaceMethod(class_, selector, (IMP)function, @encode(Type));
}

template <class Type>
bool addMethod(Class class_, SEL selector, Type function) {
	return class_addMethod(class_, selector, (IMP)function, @encode(Type));
}

void empty() {}

template <class Func>
void createHook(std::string const& className, std::string const& funcName, Func function) {
	if (auto res = ObjcHook::create(className, funcName, function, &empty)) {
		(void)Mod::get()->addHook(res.unwrap());
	}
}

void appControllerHooks() {
	// replaceMethod(class_, @selector(applicationWillBecomeActive:), &applicationWillBecomeActive);
	// replaceMethod(class_, @selector(applicationWillResignActive:), &applicationWillResignActive);
}

$execute {
	createHook("AppController", "windowWillEnterFullScreen:", &windowWillEnterFullScreen);
	createHook("AppController", "windowWillExitFullScreen:", &windowWillExitFullScreen);
	createHook("AppController", "applicationDidFinishLaunching:", &applicationDidFinishLaunching);
}