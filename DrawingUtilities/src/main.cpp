#include <Geode/Loader.hpp>
#include <LineGenerator.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

#include <Geode/modify/EditorUI.hpp>

struct LineButton : Modify<LineButton, EditorUI> {
	bool lineEnabled = false;
	Ref<CCArray> lineObjects;
	Ref<CCLayer> lineLayer;
	bool abPopulated = false;
	CCPoint lastBegin;
	CCPoint lastEnd;
	CCPoint begin;
	CCPoint end;

	CCPoint getPos(CCTouch* touch) {
		return m_editorLayer->m_objectLayer->convertToNodeSpace(
			this->convertToWorldSpace(CCDirector::get()->convertToGL(touch->getLocationInView()))
		);
	}

	void clearObjects() {
		m_fields->lineObjects->removeAllObjects();
		m_fields->lineLayer->removeAllChildren();
	}

	void generateLine() {
		this->clearObjects();

		if (m_fields->abPopulated) {
			auto generated = FillBezierLineGenerator().generate(
				m_fields->lastBegin, m_fields->lastEnd, m_fields->end, m_fields->begin, { 30 }
			);

			// auto generated = LineGenerator().generate(m_fields->begin, m_fields->end, { 15 });

			for (auto& data : generated) {
				// log::debug("object {} {}", data.position, data.rotation);
				auto obj = GameObject::createWithKey(data.id);

				obj->customSetup();
				obj->addColorSprite();
				obj->setupCustomSprites();

				obj->setPosition(data.position);
				obj->setStartPos(data.position);
				m_fields->lineObjects->addObject(obj);

				obj->setRotation(data.rotation);

				obj->m_scale = data.scale;
				obj->setRScale(1.0f);
				obj->m_isObjectRectDirty = true;
				obj->m_textureRectDirty = true;

				if (obj->m_baseColor) {
					obj->m_baseColor->m_colorID = 1011;
					obj->m_shouldUpdateColorSprite = true;
				}
				if (obj->m_detailColor) {
					obj->m_detailColor->m_colorID = 1011;
					obj->m_shouldUpdateColorSprite = true;
				}

				m_fields->lineLayer->addChild(obj);
			}
		}
	}

	bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
		if (!m_fields->lineEnabled) {
			return EditorUI::ccTouchBegan(touch, event);
		}
		m_fields->begin = this->getPos(touch);
		return true;
	}

	void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
		if (!m_fields->lineEnabled) {
			return EditorUI::ccTouchMoved(touch, event);
		}
		m_fields->end = this->getPos(touch);
		this->generateLine();
	}

	void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
		if (!m_fields->lineEnabled) {
			return EditorUI::ccTouchEnded(touch, event);
		}
		m_fields->end = this->getPos(touch);
		this->generateLine();

		for (auto obj : CCArrayExt<GameObject*>(m_fields->lineObjects)) {
			m_editorLayer->addToSection(obj);
			m_editorLayer->addSpecial(obj);
		}
		this->clearObjects();

		m_fields->lastBegin = m_fields->begin;
		if (m_fields->abPopulated) {
			m_fields->lastEnd = m_fields->begin - (m_fields->end - m_fields->begin);
		}
		else {
			m_fields->lastEnd = m_fields->end;
		}

		m_fields->abPopulated = true;
	}

	void onLineButton(CCObject* sender) {
		m_fields->abPopulated = false;
		m_fields->lineEnabled = !m_fields->lineEnabled;
	}

	bool init(LevelEditorLayer* editorLayer) {
		if (!EditorUI::init(editorLayer)) {
			return false;
		}
		m_fields->lineObjects = CCArray::create();
		m_fields->lineLayer = CCLayer::create();
		m_editorLayer->m_objectLayer->addChild(m_fields->lineLayer);

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