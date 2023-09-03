#include <Geode/Loader.hpp>
#include <LineDrawer.hpp>
#include <LineGenerator.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

#include <Geode/modify/EditorUI.hpp>

struct LineButton : Modify<LineButton, EditorUI> {
	bool lineEnabled = false;
	LineDrawer drawer;

	CCPoint getPos(CCTouch* touch) {
		return m_editorLayer->m_objectLayer->convertToNodeSpace(
			this->convertToWorldSpace(CCDirector::get()->convertToGL(touch->getLocationInView()))
		);
	}

	bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
		if (!m_fields->lineEnabled) {
			return EditorUI::ccTouchBegan(touch, event);
		}
		m_fields->drawer.began(this->getPos(touch));
		return true;
	}

	void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
		if (!m_fields->lineEnabled) {
			return EditorUI::ccTouchMoved(touch, event);
		}
		m_fields->drawer.moved(this->getPos(touch));
	}

	void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
		if (!m_fields->lineEnabled) {
			return EditorUI::ccTouchEnded(touch, event);
		}
		m_fields->drawer.ended(this->getPos(touch));
	}

	void onLineButton(CCObject* sender) {
		m_fields->lineEnabled = !m_fields->lineEnabled;
		m_fields->drawer.reset();
	}

	bool init(LevelEditorLayer* editorLayer) {
		if (!EditorUI::init(editorLayer)) {
			return false;
		}
		m_fields->drawer.setEditor(this);

		auto menu = static_cast<CCMenu*>(this->getChildByID("editor-buttons-menu"));
		if (!menu) {
			log::error("Could not find the editor button menu");
			return true;
		}

		auto lineText = CCLabelBMFont::create("Line", "bigFont.fnt");

		auto buttonSprite = EditorButtonSprite::create(lineText, EditorBaseColor::Pink);

		auto button = CCMenuItemSpriteExtra::create(
			buttonSprite, this, menu_selector(LineButton::onLineButton)
		);
		button->setSizeMult(1.2);
		button->setContentSize({ 40.0f, 40.0f });

		menu->addChild(button);
		menu->updateLayout();

		return true;
	}
};