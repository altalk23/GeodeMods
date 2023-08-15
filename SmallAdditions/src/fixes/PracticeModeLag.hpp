
#include <Geode/modify/PlayLayer.hpp>

struct PracticeModeLag : Modify<PracticeModeLag, PlayLayer> {
	int resetDelta;

	bool init(GJGameLevel* level) {
		m_fields->resetDelta = 0;
		if (!PlayLayer::init(level)) {
			return false;
		}
		return true;
	}

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

// #include <Geode/modify/PlayerObject.hpp>

// struct MAT : Modify<MAT, PlayerObject> {
// 	float f_lastPos = 0.0f;

// 	void update(float dt) {
// 		if (!PlayLayer::get()) {
// 			return PlayerObject::update(dt);
// 		}
// 		// log::debug("update!!!");
// 		auto player1 = PlayLayer::get()->m_player1;

// 		if (this == player1 && m_fields->f_lastPos != this->getPosition().x) {
// 			log::debug("difference is {}", this->getPosition().x - m_fields->f_lastPos);
// 		}

// 		PlayerObject::update(dt);

// 		if (this == player1) {
// 			m_fields->f_lastPos = this->getPosition().x;
// 		}
// 	}
// };

// #include <Geode/modify/PlayerObject.hpp>

// struct TestPlayObj : Modify<TestPlayObj, PlayerObject> {
// 	void checkSnapJumpToObject(GameObject* obj) {
// 		if (m_lastCollidedSolid) {
// 			log::debug(
// 				"last collided: {}, colliding: {}", m_lastCollidedSolid->m_uniqueID, obj->m_uniqueID
// 			);
// 			log::debug(
// 				"checkSnapJumpToObject before - {} {} {}", this->getPosition(), obj->getPosition(),
// 				m_lastCollidedSolid->getPosition()
// 			);

// 			auto x680 = cast::union_cast<double(PlayerObject::*)>((void*)0x680);

// 			// log::debug("checking {}", this->*x680);

// 			auto objPos1 = obj->getPosition();
// 			auto objPos2 = m_lastCollidedSolid->getPosition();

// 			log::debug(
// 				"boob: {}, {}", std::fabs(objPos1.x - objPos2.x + -120.00), this->getPosition().x
// 			);
// 			log::debug("also this: {}, {}", objPos1.x, objPos2.x);
// 			log::debug("mem680 value {}", this->*x680);

// 			if ((std::fabs(objPos1.x - objPos2.x + -150.00) <= 1.0 &&
// 				 std::fabs(objPos1.y - objPos2.y + -30.00) <= 1.0) ||
// 				(std::fabs(objPos1.x - objPos2.x + -195.0) <= 1.0 &&
// 				 std::fabs(objPos1.y - objPos2.y + +30.00) <= 1.0) ||
// 				(std::fabs(objPos1.x - objPos2.x + -120.00) <= 1.0 &&
// 				 std::fabs(objPos1.y - objPos2.y + -60.00) <= 1.0)) {
// 				log::debug("eligible!!!");
// 			}
// 		}

// 		auto pos = this->getPosition();

// 		// if ((std::fabs(objPos1.x - (90.0 + plaPos1.x)) <= 1.0 &&
// 		// 	 std::fabs(objPos1.y - plaPos1.y + (this->m_isUpsideDown ? 30.0 : -30.0) <= 1.0)) ||
// 		// 	(std::fabs(objPos1.x - (120.0 + plaPos1.x)) <= 1.0 &&
// 		// 	 std::fabs(objPos1.y - plaPos1.y + (this->m_isUpsideDown ? -30.0 : 30.0) <= 1.0)) ||
// 		// 	(std::fabs(objPos1.x - (60.0 + plaPos1.x)) <= 1.0 &&
// 		// 	 std::fabs(objPos1.y - plaPos1.y + (this->m_isUpsideDown ? 60.0 : -60.0) <= 1.0))) {
// 		// 	log::debug("eligible!!!");
// 		// }

// 		PlayerObject::checkSnapJumpToObject(obj);

// 		auto pos2 = this->getPosition();

// 		if (pos2.x != pos.x) {
// 			log::debug("happeed!! {} to {}", pos.x, pos2.x);
// 		}
// 		// log::debug(
// 		// 	"checkSnapJumpToObject fater - {} {} diff {}",
// 		// 	PlayLayer::get()->m_player1->getPosition(), obj->getPosition(),
// 		// 	PlayLayer::get()->m_player1->getPosition().x - obj->getPosition().x
// 		// );

// 		// this->setPosition(CCPointMake(pos.x, this->getPosition().y));
// 	}
// };