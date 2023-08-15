#include "FPSBypass.hpp"

#include <Geode/cocos/platform/mac/CCEventDispatcher.h>

using namespace geode::prelude;
using namespace tulip::fps;

FPSBypass* FPSBypass::get() {
	static FPSBypass s_ret;
	return &s_ret;
}

void FPSBypass::destroy() {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	[physicsTimer invalidate];
	physicsTimer = nil;
	if (displayLink) {
		CVDisplayLinkStop(displayLink);
		CVDisplayLinkRelease(displayLink);
		displayLink = nil;
	}

	[pool release];
}

FPSBypass::FPSBypass() :
	vsync(Mod::get()->getSettingValue<bool>("vsync")),
	updateInterval(1 / Mod::get()->getSettingValue<double>("fps")) {}

FPSBypass::~FPSBypass() {
	this->destroy();
}

void FPSBypass::updateLoop() {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	struct cc_timeval now;
	static cocos2d::cc_timeval last = {};
	float newDelta;
	if (last.tv_sec == 0 && last.tv_usec == 0) {
		CCTime::gettimeofdayCocos2d(&last, nullptr);
	}

	CCTime::gettimeofdayCocos2d(&now, nullptr);

	newDelta = (now.tv_sec - last.tv_sec) + (now.tv_usec - last.tv_usec) / 1000000.0f;
	newDelta = std::max(0.0f, newDelta);

	last = now;

	NSOpenGLView* openGLView = [NSClassFromString(@"EAGLView") sharedEGLView];
	NSOpenGLContext* glContext = [openGLView openGLContext];

	[glContext makeCurrentContext];
	CGLLockContext([glContext CGLContextObj]);

	CCDirector::sharedDirector()->getScheduler()->update(newDelta);

	CGLUnlockContext([glContext CGLContextObj]);

	[pool release];
}

void FPSBypass::displayLoop(bool paused) {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	// get the opengl view
	NSOpenGLView* openGLView = [NSClassFromString(@"EAGLView") sharedEGLView];
	NSOpenGLContext* glContext = [openGLView openGLContext];

	[glContext makeCurrentContext];
	CGLLockContext([glContext CGLContextObj]);

	[(id)[NSClassFromString(@"CCEventDispatcher") sharedDispatcher] dispatchQueuedEvents];

	cocos2d::CCDirector::sharedDirector()->setPaused(paused);
	CCDirector::sharedDirector()->drawScene();
	cocos2d::CCDirector::sharedDirector()->setPaused(false);

	// CCPoolManager::sharedPoolManager()->pop();

	[glContext flushBuffer];

	CGLUnlockContext([glContext CGLContextObj]);

	[pool release];
}

static CVReturn DisplayLinkCallback(
	CVDisplayLinkRef displayLink, CVTimeStamp const* now, CVTimeStamp const* outputTime,
	CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext
) {
	FPSBypass::get()->displayLoop(!FPSBypass::get()->vsync);
	return kCVReturnSuccess;
}

void FPSBypass::setVSync(bool value) {
	CCDirector::sharedDirector()->setAnimationInterval(updateInterval);
}

void FPSBypass::setAnimationInterval(double interval) {
	[physicsTimer invalidate];
	physicsTimer = nil;

	if (!vsync) {
		auto thing = ^void(NSTimer* timer) { updateLoop(); };

		physicsTimer = [NSTimer timerWithTimeInterval:interval repeats:YES block:thing];
		[[NSRunLoop currentRunLoop] addTimer:physicsTimer forMode:NSDefaultRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer:physicsTimer forMode:NSEventTrackingRunLoopMode];
	}
}

void FPSBypass::startMainLoop() {
	CVDisplayLinkRelease(displayLink);
	displayLink = nil;

	NSOpenGLView* openGLView = [NSClassFromString(@"EAGLView") sharedEGLView];
	NSOpenGLContext* glContext = [openGLView openGLContext];

	GLint swapInterval = 0;
	[glContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	CVDisplayLinkSetOutputCallback(displayLink, &DisplayLinkCallback, openGLView);

	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(
		displayLink, [glContext CGLContextObj], [[openGLView pixelFormat] CGLPixelFormatObj]
	);
	CVDisplayLinkStart(displayLink);

	CCDirector::sharedDirector()->setAnimationInterval(updateInterval);
}
