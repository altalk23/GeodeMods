
#include <Geode/modify/PlayLayer.hpp>

struct PracticeModeLag : Modify<PracticeModeLag, PlayLayer> {
	int resetDelta = 0;

	void update(float dt) override {
		if (m_fields->resetDelta == 0) {
			return PlayLayer::update(dt);
		}

		float time = 0.0001;
		if (m_fields->resetDelta != 0 && dt - time < 1) { // if close enough to normal speed
			m_fields->resetDelta -= 1;
		}

		return PlayLayer::update(time);
	}

	void resetLevel() {
		if (m_isPracticeMode || m_isTestMode) {
			m_fields->resetDelta = 2; // Account for 1 extra frame respawn
		}
		PlayLayer::resetLevel();
	}
};

// #include <Geode/modify/CCFileUtils.hpp>

// struct File : Modify<File, cocos2d::CCFileUtils> {
// 	virtual gd::string fullPathForFilename(char const* filename, bool unk) {
// 		log::debug("looking for {}", filename);

// 		auto ret = CCFileUtils::fullPathForFilename(filename, unk);

// 		auto val = m_fullPathCache.std();

// 		log::debug("in cache: {}, {}", val.begin()->first, val.begin()->second);
// 		for (auto it = val.begin(); it != val.end(); ++it) {
// 			log::debug("in cache: {}, {}", it->first, it->second);
// 		}

// 		log::debug("cache size {}", val.size());

// 		return ret;
// 	}
// };