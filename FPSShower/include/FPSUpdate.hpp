#pragma once

#include <FPSOverlay.hpp>
#include <cocos-ext.h>
#include <cocos2d.h>

namespace tulip::fps {

	class FPSUpdate : public cocos2d::CCNode {
	protected:
		int m_frames = 0;
		double m_accumulate = 0.0;
		double m_resetInterval = 1.0;
		double m_framerate = 0.0;
		geode::Ref<FPSOverlay> m_overlay;

	public:
		static FPSUpdate* get();

		void setOverlay(FPSOverlay* overlay);
		void releaseOverlay();
		double getFramerate();

		bool init() override;
		void update(float dt) override;
	};
}