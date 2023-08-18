#import <Cocoa/Cocoa.h>
#include <Geode/Bindings.hpp>
#include <Geode/DefaultInclude.hpp>
#include <Geode/Loader.hpp>
#import <Geode/cocos/platform/mac/EAGLView.h>
#import <objc/runtime.h>

using namespace geode::prelude;

#include <Geode/modify/PlatformToolbox.hpp>

struct ToggleFullscreenReplace : Modify<ToggleFullscreenReplace, PlatformToolbox> {
	static void toggleFullScreen(bool fullscreen) {
		auto window = [[NSApplication sharedApplication] mainWindow];

		[window toggleFullScreen:nil];
	}
};

static IMP s_applicationDidFinishLaunching;

void applicationDidFinishLaunching(id self, SEL selector, NSNotification* notification) {
	log::debug("dfgsjkhjgdfshjgksjdfkgjhskjdgf");
	auto windowed = GameManager::get()->getGameVariable("0025");
	if (!windowed) {
		GameManager::get()->setGameVariable("0025", true);
	}
	using Type = decltype(&applicationDidFinishLaunching);
	reinterpret_cast<Type>(s_applicationDidFinishLaunching)(self, selector, notification);

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
	auto windowed = GameManager::get()->getGameVariable("0025");
	if (windowed) {
		AppDelegate::get()->applicationWillEnterForeground();
	}
	else {
		AppDelegate::get()->applicationWillBecomeActive();
	}
}

void applicationWillResignActive(id self, SEL selector, NSNotification* notification) {
	auto windowed = GameManager::get()->getGameVariable("0025");
	if (windowed) {
		AppDelegate::get()->applicationDidEnterBackground();
	}
	else {
		AppDelegate::get()->applicationWillResignActive();
	}
}

template <class Type>
IMP replaceMethod(Class class_, SEL selector, Type function) {
	return class_replaceMethod(class_, selector, (IMP)function, @encode(Type));
}

template <class Type>
bool addMethod(Class class_, SEL selector, Type function) {
	return class_addMethod(class_, selector, (IMP)function, @encode(Type));
}

void appControllerHooks() {
	Class class_ = objc_getClass("AppController");

	addMethod(class_, @selector(windowWillEnterFullScreen:), &windowWillEnterFullScreen);
	addMethod(class_, @selector(windowWillExitFullScreen:), &windowWillExitFullScreen);

	s_applicationDidFinishLaunching = replaceMethod(
		class_, @selector(applicationDidFinishLaunching:), &applicationDidFinishLaunching
	);
	replaceMethod(class_, @selector(applicationWillBecomeActive:), &applicationWillBecomeActive);
	replaceMethod(class_, @selector(applicationWillResignActive:), &applicationWillResignActive);
}

$execute {
	appControllerHooks();
}