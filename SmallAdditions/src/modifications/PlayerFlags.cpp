#include "PlayerFlags.hpp"

using namespace geode::prelude;

inline bool shouldFlag() {
	int chance = std::rand() % 1000;
	return chance <= 58;
}

static bool s_inCommentCell = false;

#include <Geode/modify/CommentCell.hpp>

struct CommentCellCheck : Modify<CommentCellCheck, CommentCell> {
	void loadFromComment(GJComment* comment) {
		s_inCommentCell = true;
		CommentCell::loadFromComment(comment);
		s_inCommentCell = false;
	}
};

inline auto getRandomFlag() {
	static std::array<char const*, 17> array = {
		"agenderFlag.png"_spr,     "aromanticFlag.png"_spr,   "asexualFlag.png"_spr,
		"bigenderFlag.png"_spr,    "bisexualFlag.png"_spr,    "demigenderFlag.png"_spr,
		"demisexualFlag.png"_spr,  "enbyFlag.png"_spr,        "gayFlag.png"_spr,
		"genderfluidFlag.png"_spr, "genderqueerFlag.png"_spr, "intersexFlag.png"_spr,
		"lesbianFlag.png"_spr,     "pansexualFlag.png"_spr,   "polysexualFlag.png"_spr,
		"rainbowFlag.png"_spr,     "transgenderFlag.png"_spr
	};

	return array[std::rand() % array.size()];
}

inline void addFlagForSprite(
	CCNode* parent, CCSprite* iconSprite, float sizeMultiplier, CCPoint const& iconOffset
) {
	// Log::get() << parent << " " << iconSprite << " " <<sizeMultiplier << " " <<iconOffset;
	// auto clippingNode = CCClippingNode::create();
	// clippingNode->setAlphaThreshold(0.4);
	// clippingNode->setStencil(iconSprite);
	// Log::get() << clippingNode;
	// parent->addChild(clippingNode, 1);
	// Log::get() << clippingNode;

	// Log::get() << getRandomFlag();
	auto overlaySprite = CCSprite::createWithSpriteFrameName(getRandomFlag());
	// overlaySprite->setBlendFunc({GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA}); //overlay
	// overlaySprite->setPosition((iconSprite->getPosition() + iconOffset) /
	// 	parent->getScaleY()
	// );
	parent->addChild(overlaySprite, 5);

	auto overlaySize = overlaySprite->getContentSize();
	auto iconSize = iconSprite->getContentSize() * sizeMultiplier;
	overlaySprite->setScaleX(iconSize.width / overlaySize.width);
	overlaySprite->setScaleY(iconSize.height / overlaySize.height);
}

#include <Geode/modify/SimplePlayer.hpp>

struct SimplePlayerFlags : Modify<SimplePlayerFlags, SimplePlayer> {
	CCClippingNode* clippingNode;
	CCSprite* overlaySprite;
	bool hasFlag;

	inline void updateOverlayForIcon(CCNode* icon, CCSize const& iconSize) {
		auto clippingNode = m_fields->clippingNode;
		auto overlaySprite = m_fields->overlaySprite;

		clippingNode->setStencil(icon);
		auto overlaySize = overlaySprite->getContentSize();
		overlaySprite->setScaleX(iconSize.width / overlaySize.width);
		overlaySprite->setScaleY(iconSize.height / overlaySize.height);
	}

	bool init(int iconID) {
		if (!SimplePlayer::init(iconID)) {
			return false;
		}

		if (!Mod::get()->getSettingValue<bool>("player-flags")) {
			return true;
		}

		m_fields->hasFlag = s_inCommentCell && shouldFlag();
		if (!m_fields->hasFlag) {
			return true;
		}

		auto clippingNode = CCClippingNode::create();
		clippingNode->setAlphaThreshold(0.4);
		this->addChild(clippingNode, 1);
		m_fields->clippingNode = clippingNode;

		auto overlaySprite = CCSprite::createWithSpriteFrameName(getRandomFlag());
		overlaySprite->setBlendFunc({ GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA }); // overlay
		m_fields->overlaySprite = overlaySprite;

		this->updateOverlayForIcon(m_firstLayer, m_firstLayer->getContentSize());

		clippingNode->addChild(overlaySprite);
		return true;
	}

	void updatePlayerFrame(int iconID, IconType iconType) {
		SimplePlayer::updatePlayerFrame(iconID, iconType);
		if (!Mod::get()->getSettingValue<bool>("player-flags")) {
			return;
		}

		if (!m_fields->hasFlag) {
			return;
		}

		auto clippingNode = m_fields->clippingNode;
		auto overlaySprite = m_fields->overlaySprite;

		if (!(clippingNode && overlaySprite)) {
			return;
		}

		switch (iconType) {
			case IconType::Robot:
				updateOverlayForIcon(m_robotSprite, m_robotSprite->getContentSize() * 1.25);
				overlaySprite->setPosition(
					(m_robotSprite->getPosition() + ccp(0, 2.5)) / clippingNode->getScaleY()
				);
				// updateOverlayForIcon(m_firstLayer);
				break;

			case IconType::Spider:
				updateOverlayForIcon(m_spiderSprite, m_spiderSprite->getContentSize() * 1.5);
				overlaySprite->setPosition(
					(m_spiderSprite->getPosition() + ccp(0, 5)) / clippingNode->getScaleY()
				);
				// updateOverlayForIcon(m_firstLayer);
				break;

			default:
				updateOverlayForIcon(m_firstLayer, m_firstLayer->getContentSize());
				overlaySprite->setPosition(m_firstLayer->getPosition() / clippingNode->getScaleY());
				break;
		}
	}

	void updateColors() {
		if (m_fields->hasFlag) {
			m_hasGlowOutline = false;
			m_firstLayer->setColor(ccc3(255, 255, 255));
			m_secondLayer->setColor(ccc3(255, 255, 255));
		}
		SimplePlayer::updateColors();
	}
};

// FlagPlayerObject* FlagPlayerObject::create(int p0, int p1, cocos2d::CCLayer* p2) {
// 	auto ret = new FlagPlayerObject();
// 	if (ret && ret->init(p0, p1, p2)) {
// 		ret->autorelease();
// 		return ret;
// 	}
// 	return nullptr;
// }

// void FlagPlayerObject::updateFlags() {
// 	CCSprite* iconSprite;
// 	CCSize iconSize;
// 	CCPoint iconOffset;
// 	CCSize overlaySize;
// 	if (m_isRobot) {
// 		iconSprite = m_robotSprite;
// 		iconSize = iconSprite->getContentSize() * 1.25;
// 		iconOffset = CCPointMake(0, 2.5);
// 		m_iconClip->setZOrder(3);
//  	}
//  	else if (m_isSpider) {
//  		iconSprite = m_spiderSprite;
//  		iconSize = iconSprite->getContentSize() * 1.5;
//  		iconOffset = CCPointMake(0, 5);
//  		m_iconClip->setZOrder(3);
//  	}
//  	else {
// 		iconSprite = m_iconSprite;
// 		iconSize = iconSprite->getContentSize();
// 		iconOffset = CCPointMake(0, 0);
// 		m_iconClip->setZOrder(1);
// 	}

//  	m_iconClip->setStencil(iconSprite);

// 	overlaySize = m_iconFlag->getContentSize();
// 	m_iconFlag->setScaleX(iconSize.width / overlaySize.width);
// 	m_iconFlag->setScaleY(iconSize.height / overlaySize.height);

// 	m_iconFlag->setPosition((iconSprite->getPosition() + iconOffset) /
// 		m_iconClip->getScaleY()
// 	);

// 	if (m_isShip || m_isBird) {
// 		m_vehicleClip->setStencil(m_vehicleSprite);

// 		auto vehicleSize = m_vehicleSprite->getContentSize();
// 		auto overlaySize = m_vehicleFlag->getContentSize();
// 		m_vehicleFlag->setScaleX(vehicleSize.width / overlaySize.width);
// 		m_vehicleFlag->setScaleY(vehicleSize.height / overlaySize.height);

// 		m_vehicleFlag->setPosition((m_vehicleSprite->getPosition()) /
// 			m_vehicleClip->getScaleY()
// 		);
// 	}
// }

// void FlagPlayerObject::changeCurrentFlag() {
// 	m_iconClip->removeAllChildren();
// 	m_vehicleClip->removeAllChildren();

// 	auto selectedFlag = getRandomFlag();
// 	m_iconFlag = CCSprite::createWithSpriteFrameName(selectedFlag);
// 	m_iconFlag->setBlendFunc({GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
// 	m_iconClip->addChild(m_iconFlag);
// 	m_vehicleFlag = CCSprite::createWithSpriteFrameName(selectedFlag);
// 	m_vehicleFlag->setBlendFunc({GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA});
// 	m_vehicleClip->addChild(m_vehicleFlag);
// }

// bool FlagPlayerObject::init(int p0, int p1, cocos2d::CCLayer* p2) {
// 	if (!PlayerObject::init(p0, p1, p2)) return false;
// 	m_iconClip = CCClippingNode::create(m_iconSprite);
// 	m_iconClip->setAlphaThreshold(0.4);
// 	addChild(m_iconClip, 1);
// 	m_vehicleClip = CCClippingNode::create(m_iconSprite);
// 	m_vehicleClip->setAlphaThreshold(0.4);
// 	addChild(m_vehicleClip, 3);

// 	return true;
// }

// static bool s_inMenuGameLayer = false;

// class $modify(MenuGameLayer) {
// 	bool init() {
// 		s_inMenuGameLayer = true;

// 		if (!MenuGameLayer::init()) return false;
// 		if (shouldFlag()) {
// 			m_player->setColor(ccc3(255, 255, 255));
// 			m_player->setSecondColor(ccc3(255, 255, 255));
// 			static_cast<FlagPlayerObject*>(m_player)->changeCurrentFlag();
// 			static_cast<FlagPlayerObject*>(m_player)->updateFlags();
// 		}
// 		else {
// 			static_cast<FlagPlayerObject*>(m_player)->m_iconClip->removeAllChildren();
// 			static_cast<FlagPlayerObject*>(m_player)->m_vehicleClip->removeAllChildren();
// 		}

// 		s_inMenuGameLayer = false;
// 		return true;
// 	}

// 	void resetPlayer() {
// 		MenuGameLayer::resetPlayer();
// 		if (m_player->m_isSpider) {
// 			// bugfix
// 			m_player->updatePlayerSpiderFrame((std::rand() % 26)+1);
// 		}
// 		if (shouldFlag()) {
// 			m_player->setColor(ccc3(255, 255, 255));
// 			m_player->setSecondColor(ccc3(255, 255, 255));
// 			static_cast<FlagPlayerObject*>(m_player)->changeCurrentFlag();
// 			static_cast<FlagPlayerObject*>(m_player)->updateFlags();
// 		}
// 		else {
// 			static_cast<FlagPlayerObject*>(m_player)->m_iconClip->removeAllChildren();
// 			static_cast<FlagPlayerObject*>(m_player)->m_vehicleClip->removeAllChildren();
// 		}

// 	}
// };

// class $modify(PlayerObject) {
// 	static PlayerObject* create(int p0, int p1, cocos2d::CCLayer* p2) {
// 		if (s_inMenuGameLayer) return FlagPlayerObject::create(p0, p1, p2);
// 		return PlayerObject::create(p0, p1, p2);
// 	}
// };
