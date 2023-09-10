#include "FPSBypass.hpp"

#include <Geode/cocos/platform/mac/CCDirectorCaller.h>
#include <Geode/cocos/platform/mac/EAGLView.h>
#include <Geode/loader/SettingEvent.hpp>
#import <objc/runtime.h>

using namespace geode::prelude;
using namespace tulip::fps;

void setAnimationInterval(CCDirectorCaller* self, SEL sel, double interval) {
	if (interval == 0.25) {
		return CCDirector::get()->setAnimationInterval(
			1.0 / Mod::get()->getSettingValue<double>("fps")
		);
	}
	FPSBypass::get()->setAnimationInterval(interval);
}

void startMainLoop(CCDirectorCaller*, SEL) {
	FPSBypass::get()->startMainLoop();
}

template <class Return = bool>
Return wrapOpenGLContextDestroy(id self, SEL sel, NSEvent* event) {
	FPSBypass::get()->destroy();

	NSOpenGLView* openGLView = [NSClassFromString(@"EAGLView") sharedEGLView];
	NSOpenGLContext* glContext = [openGLView openGLContext];

	[glContext makeCurrentContext];
	CGLLockContext([glContext CGLContextObj]);

	NSInvocation* invocation = [NSInvocation
		invocationWithMethodSignature:[[self class] instanceMethodSignatureForSelector:sel]];
	[invocation setSelector:sel];
	[invocation setTarget:self];
	[invocation invoke];
	[invocation setArgument:event atIndex:2];
	Return returnValue;
	[invocation getReturnValue:&returnValue];

	CGLUnlockContext([glContext CGLContextObj]);

	return returnValue;
}

void wrapOpenGLContext(id self, SEL sel, NSEvent* event) {
	NSOpenGLView* openGLView = [NSClassFromString(@"EAGLView") sharedEGLView];
	NSOpenGLContext* glContext = [openGLView openGLContext];

	[glContext makeCurrentContext];
	CGLLockContext([glContext CGLContextObj]);

	[self performSelector:sel withObject:event];

	CGLUnlockContext([glContext CGLContextObj]);
}

void empty() {}

template <class Func>
void createHook(std::string const& className, std::string const& funcName, Func function) {
	if (auto res = ObjcHook::create(className, funcName, function, &empty)) {
		(void)Mod::get()->addHook(res.unwrap());
	}
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

	// createHook("AppController", "applicationShouldTerminate:", &applicationShouldTerminate);

	createHook("AppController", "windowShouldClose:", &wrapOpenGLContextDestroy<bool>);
	createHook("AppController", "applicationShouldTerminate:", &wrapOpenGLContextDestroy<NSApplicationTerminateReply>);

	createHook("CCDirectorCaller", "setAnimationInterval:", &setAnimationInterval);
	createHook("CCDirectorCaller", "startMainLoop", &startMainLoop);

	createHook("EAGLView", "mouseDown:", &wrapOpenGLContext);
	createHook("EAGLView", "mouseUp:", &wrapOpenGLContext);
	createHook("EAGLView", "mouseMoved:", &wrapOpenGLContext);
	createHook("EAGLView", "mouseDragged:", &wrapOpenGLContext);
	createHook("EAGLView", "rightMouseDown:", &wrapOpenGLContext);
	createHook("EAGLView", "rightMouseDragged:", &wrapOpenGLContext);
	createHook("EAGLView", "rightMouseUp:", &wrapOpenGLContext);
	createHook("EAGLView", "otherMouseDown:", &wrapOpenGLContext);
	createHook("EAGLView", "otherMouseDragged:", &wrapOpenGLContext);
	createHook("EAGLView", "otherMouseUp:", &wrapOpenGLContext);
	createHook("EAGLView", "scrollWheel:", &wrapOpenGLContext);
	createHook("EAGLView", "mouseEntered:", &wrapOpenGLContext);
	createHook("EAGLView", "mouseExited:", &wrapOpenGLContext);

	createHook("EAGLView", "keyDown:", &wrapOpenGLContext);
	createHook("EAGLView", "keyUp:", &wrapOpenGLContext);
	createHook("EAGLView", "flagsChanged:", &wrapOpenGLContext);

	createHook("EAGLView", "touchesBeganWithEvent:", &wrapOpenGLContext);
	createHook("EAGLView", "touchesMovedWithEvent:", &wrapOpenGLContext);
	createHook("EAGLView", "touchesEndedWithEvent:", &wrapOpenGLContext);
	createHook("EAGLView", "touchesCancelledWithEvent:", &wrapOpenGLContext);
}
