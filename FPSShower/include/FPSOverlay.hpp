#pragma once

#include <cocos-ext.h>
#include <cocos2d.h>

namespace tulip::fps {
	class FPSOverlay : public cocos2d::CCNode {
	protected:
		cocos2d::CCLabelBMFont* m_label;
		cocos2d::extension::CCScale9Sprite* m_backdrop;

	public:
		static FPSOverlay* create();
		bool init() override;

		void setFramerate(double framerate);

		cocos2d::CCSize getBackdropSize();
	};

}