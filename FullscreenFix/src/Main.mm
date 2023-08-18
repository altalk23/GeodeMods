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

void applicationDidFinishLaunching(id self, SEL sel, NSNotification* notification) {
	auto windowed = GameManager::get()->getGameVariable("0025");
	if (!windowed) {
		GameManager::get()->setGameVariable("0025", true);
	}
	[(id)self applicationDidFinishLaunching:notification];

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

$execute {
	(void)Mod::get()->addHook(Hook::create(
		Mod::get(), (void*)(base::get() + 0x69a0), &applicationDidFinishLaunching,
		"AppController::applicationDidFinishLaunching:", tulip::hook::TulipConvention::Default
	));

	Class metaclass = (Class)((uintptr_t)objc_getMetaClass("AppController") - 0x28);

	auto res = class_addMethod(
		metaclass, @selector(windowWillEnterFullScreen:), (IMP)&windowWillEnterFullScreen,
		"v24@0:8@16"
	);
	log::debug("Create AppController::windowWillEnterFullScreen:");

	res = class_addMethod(
		metaclass, @selector(windowWillExitFullScreen:), (IMP)&windowWillExitFullScreen,
		"v24@0:8@16"
	);
	log::debug("Create AppController::windowWillExitFullScreen:");
}