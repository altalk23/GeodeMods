#pragma once

#import <Cocoa/Cocoa.h>
#include <Geode/cocos/platform/mac/EAGLView.h>
#include <Geode/loader/Loader.hpp>

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