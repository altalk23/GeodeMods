
#include <Geode/modify/PlayLayer.hpp>

struct LowFPSKick : Modify<LowFPSKick, PlayLayer> {
	void update(float dt) {
		m_shouldTryToKick = false;
		PlayLayer::update(dt);
	}
};
