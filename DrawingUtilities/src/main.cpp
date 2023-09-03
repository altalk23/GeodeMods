#include <Geode/Loader.hpp>
#include <LineDrawer.hpp>
#include <LineGenerator.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

#include <Geode/modify/EditorUI.hpp>

struct LineButton : Modify<LineButton, EditorUI> {
	bool lineEnabled = false;
	LineDrawer drawer;
	int state = 0;
	CCMenuItemSpriteExtra* button;

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

	void updateButtonState() {
		struct Value {
			std::string text;
			EditorBaseColor color;
		};

		std::array<Value, 5> thing = { Value { "Line\nDisabled", EditorBaseColor::Gray },
									   Value { "Line", EditorBaseColor::Pink },
									   Value { "Rounded\nLine", EditorBaseColor::Green },
									   Value { "Bezier", EditorBaseColor::Aqua } };

		auto state = m_fields->state;
		auto lineText = CCLabelBMFont::create(thing[state].text.c_str(), "bigFont.fnt");
		lineText->setAlignment(kCCTextAlignmentCenter);
		auto buttonSprite = EditorButtonSprite::create(lineText, thing[state].color);

		m_fields->button->setNormalImage(buttonSprite);
		m_fields->button->setSelectedImage(buttonSprite);
	}

	void onLineButton(CCObject* sender) {
		m_fields->lineEnabled = true;
		m_fields->drawer.reset();
		m_fields->state++;
		if (m_fields->state == 4) {
			m_fields->state = 0;
			m_fields->lineEnabled = false;
		}
		else {
			m_fields->drawer.m_used = static_cast<UsedGenerator>(m_fields->state - 1);
		}
		this->updateButtonState();
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

		auto buttonSprite = EditorButtonSprite::create(nullptr, EditorBaseColor::Gray);

		auto button = CCMenuItemSpriteExtra::create(
			buttonSprite, this, menu_selector(LineButton::onLineButton)
		);
		m_fields->button = button;
		this->updateButtonState();

		button->setSizeMult(1.2);
		button->setContentSize({ 40.0f, 40.0f });

		menu->addChild(button);
		menu->updateLayout();

		return true;
	}
};