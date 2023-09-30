#pragma once

#import <Cocoa/Cocoa.h>
#include <Geode/Geode.hpp>
#import <QuartzCore/CVDisplayLink.h>
#include <thread>

namespace tulip::fps {
	class FPSBypass {
	public:
		CVDisplayLinkRef displayLink;
		NSTimer* physicsTimer;
		bool vsync;
		double updateInterval;

		static FPSBypass* get();

		FPSBypass();

		void updateLoop();
		void displayLoop(bool paused);

		void setVSync(bool value);
		void setAnimationInterval(double value);
		void startMainLoop();

		void destroy();

		~FPSBypass();
	};
}