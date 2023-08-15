
#include <Geode/modify/PlayLayer.hpp>

struct PracticeSongChange : Modify<PracticeSongChange, PlayLayer> {
	void startMusic() {
		if (!Mod::get()->getSettingValue<bool>("practice-song-hack")) {
			return PlayLayer::startMusic();
		}

		auto oldValue = m_isPracticeMode;
		m_isPracticeMode = false;
		PlayLayer::startMusic();
		m_isPracticeMode = oldValue;
	}

	void togglePracticeMode(bool isPractice) {
		if (!Mod::get()->getSettingValue<bool>("practice-song-hack")) {
			return PlayLayer::togglePracticeMode(isPractice);
		}

		if (!m_isPracticeMode && isPractice) {
			m_isPracticeMode = isPractice;
			m_UILayer->toggleCheckpointsMenu(isPractice);
			this->startMusic();
			if (m_isPracticeMode) {
				this->stopActionByTag(0x12);
			}
		}
		else {
			PlayLayer::togglePracticeMode(isPractice);
		}
	}
};

#include <Geode/modify/PlayerObject.hpp>

struct PlayerDeathDetect : Modify<PlayerDeathDetect, PlayerObject> {
	void playerDestroyed(bool isPlayer2) {
		if (!Mod::get()->getSettingValue<bool>("practice-song-hack")) {
			return PlayerObject::playerDestroyed(isPlayer2);
		}

		PlayerObject::playerDestroyed(isPlayer2);
		GameSoundManager::sharedManager()->stopBackgroundMusic();
	}
};