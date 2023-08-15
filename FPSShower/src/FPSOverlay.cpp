#include <FPSOverlay.hpp>
#include <Geode/Loader.hpp>
#include <sstream>

using namespace geode::prelude;
using namespace tulip::fps;

FPSOverlay* FPSOverlay::create() {
	auto ret = new (std::nothrow) FPSOverlay;
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

cocos2d::CCSize FPSOverlay::getBackdropSize() {
	auto textSize = m_label->getContentSize();
	return cocos2d::CCSizeMake(2 * 6 + textSize.width * 2, 2 * 6 + textSize.height * 2);
}

bool FPSOverlay::init() {
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	m_label = cocos2d::CCLabelBMFont::create("No FPS", "chatFont.fnt");
	m_label->setPosition({ 3, winSize.height - 3 });
	m_label->setAnchorPoint(cocos2d::CCPoint(0, 1));
	m_label->setScale(0.4);
	m_label->setColor(Mod::get()->getSettingValue<ccColor3B>("text-color"));
	this->addChild(m_label, 2);

	m_backdrop = cocos2d::extension::CCScale9Sprite::create("square02_small.png");
	m_backdrop->setContentSize(getBackdropSize());
	m_backdrop->setPosition({ 2, winSize.height - 2 });
	m_backdrop->setAnchorPoint(cocos2d::CCPoint(0, 1));
	m_backdrop->setOpacity(127);
	m_backdrop->setScale(0.2);
	this->addChild(m_backdrop, 1);

	return true;
}

void FPSOverlay::setFramerate(double framerate) {
	std::stringstream stream;
	stream << framerate << " FPS";

	m_label->setString(stream.str().c_str());
	m_backdrop->setContentSize(getBackdropSize());
}
