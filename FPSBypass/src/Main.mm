#include "FPSBypass.hpp"

#include <Geode/cocos/platform/mac/CCDirectorCaller.h>
#include <Geode/cocos/platform/mac/EAGLView.h>
#include <Geode/loader/SettingEvent.hpp>
#import <objc/runtime.h>

using namespace geode::prelude;
using namespace tulip::fps;

void setAnimationInterval(CCDirectorCaller*, SEL, double interval) {
	log::debug("set animation interval {}", interval);
	FPSBypass::get()->setAnimationInterval(interval);
}

void startMainLoop(CCDirectorCaller*, SEL) {
	FPSBypass::get()->startMainLoop();
}

static IMP s_applicationShouldTerminate;

NSApplicationTerminateReply applicationShouldTerminate(
	void* controller, SEL sel, NSApplication* sender
) {
	using Type = decltype(&applicationShouldTerminate);
	FPSBypass::get()->destroy();
	return reinterpret_cast<Type>(s_applicationShouldTerminate)(controller, sel, sender);
}

using EventType = void (*)(id, SEL, NSEvent*);

template <int Value>
static IMP s_eventImplementation;

template <int Value>
void wrapOpenGLContext(id self, SEL sel, NSEvent* event) {
	NSOpenGLView* openGLView = [NSClassFromString(@"EAGLView") sharedEGLView];
	NSOpenGLContext* glContext = [openGLView openGLContext];

	[glContext makeCurrentContext];
	CGLLockContext([glContext CGLContextObj]);

	reinterpret_cast<EventType>(s_eventImplementation<Value>)(self, sel, event);

	CGLUnlockContext([glContext CGLContextObj]);
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

	s_applicationShouldTerminate =
		replaceMethod(class_, @selector(applicationShouldTerminate:), &applicationShouldTerminate);
}

void directorCallerHooks() {
	Class class_ = objc_getClass("CCDirectorCaller");

	replaceMethod(class_, @selector(setAnimationInterval:), &setAnimationInterval);
	replaceMethod(class_, @selector(startMainLoop), &startMainLoop);
}

void eaglViewHooks() {
	Class class_ = objc_getClass("EAGLView");

#define EAGL_HOOK(Id_, Sel_) \
	s_eventImplementation<Id_> = replaceMethod(class_, @selector(Sel_), &wrapOpenGLContext<Id_>);

	// Mouse
	EAGL_HOOK(0, mouseDown:);
	EAGL_HOOK(1, mouseUp:);
	EAGL_HOOK(2, mouseMoved:);
	EAGL_HOOK(3, mouseDragged:);
	EAGL_HOOK(4, rightMouseDown:);
	EAGL_HOOK(5, rightMouseDragged:);
	EAGL_HOOK(6, rightMouseUp:);
	EAGL_HOOK(7, otherMouseDown:);
	EAGL_HOOK(8, otherMouseDragged:);
	EAGL_HOOK(9, otherMouseUp:);
	EAGL_HOOK(10, scrollWheel:);
	EAGL_HOOK(11, mouseEntered:);
	EAGL_HOOK(12, mouseExited:);

	EAGL_HOOK(13, keyDown:);
	EAGL_HOOK(14, keyUp:);
	EAGL_HOOK(15, flagsChanged:);

	EAGL_HOOK(16, touchesBeganWithEvent:);
	EAGL_HOOK(17, touchesMovedWithEvent:);
	EAGL_HOOK(18, touchesEndedWithEvent:);
	EAGL_HOOK(19, touchesCancelledWithEvent:);
}

static auto fpsEvent = listenForSettingChanges(
	"fps",
	+[](double value) {
		FPSBypass::get()->updateInterval = 1 / value;
		FPSBypass::get()->setAnimationInterval(1 / value);
	}
);

static auto vsyncEvent = listenForSettingChanges(
	"vsync",
	+[](bool value) {
		FPSBypass::get()->vsync = value;
		FPSBypass::get()->setVSync(value);
	}
);

@interface CCDirectorCallerMake : NSObject {
@public
	CVDisplayLinkRef displayLink;
	NSTimer* renderTimer;
	int interval;
}
@end

@implementation CCDirectorCallerMake
@end

$on_mod(Enabled) {
	CCDirectorCallerMake* caller = [NSClassFromString(@"CCDirectorCaller") sharedDirectorCaller];
	[caller->renderTimer invalidate];

	appControllerHooks();
	directorCallerHooks();
	eaglViewHooks();

	FPSBypass::get()->startMainLoop();
}
