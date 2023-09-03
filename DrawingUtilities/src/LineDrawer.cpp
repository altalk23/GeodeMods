#include <LineDrawer.hpp>
#include <LineGenerator.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

void LineDrawer::clearObjects() {
	for (auto obj : CCArrayExt<GameObject*>(m_lineObjects)) {
		m_lineLayer->removeChild(obj);
	}
	m_lineObjects->removeAllObjects();
}

bool LineDrawer::shouldDraw() {
	using enum UsedGenerator;
	switch (m_used) {
		case Bezier:
		case FillBezier: return m_dragCount > 0;
		default: return true;
	}
}

bool LineDrawer::isContinuing() {
	using enum UsedGenerator;
	switch (m_used) {
		case Bezier:
		case FillBezier: return m_dragCount > 0;
		default: return false;
	}
}

void LineDrawer::setLastPoints() {
	using enum UsedGenerator;
	switch (m_used) {
		case Bezier:
		case FillBezier: {
			m_lastBegin = m_begin;
			m_lastEnd = m_end;
			if (this->isContinuing()) {
				m_lastEnd = m_begin - (m_end - m_begin);
			}
		} break;

		default: break;
	}
}

std::vector<ObjectData> LineDrawer::generate() {
	using enum UsedGenerator;
	std::unique_ptr<LineGenerator> generator;
	std::vector<CCPoint> points;

	switch (m_used) {
		case Line: {
			generator = std::make_unique<LineGenerator>();
			points = { m_begin, m_end };
		} break;

		case RoundedLine: {
			generator = std::make_unique<RoundedLineGenerator>();
			points = { m_begin, m_end };
		} break;

		case Bezier: {
			generator = std::make_unique<BezierLineGenerator>();
			points = { m_lastBegin, m_lastEnd, m_end, m_begin };
		} break;

		case FillBezier: {
			generator = std::make_unique<FillBezierLineGenerator>();
			points = { m_lastBegin, m_lastEnd, m_end, m_begin };
		} break;

		default: return {};
	}

	return generator->generate(points, { m_thickness, this->isContinuing() });
}

void LineDrawer::drawOverlay() {
	using enum UsedGenerator;
	m_drawLayer->clear();

	switch (m_used) {
		case Line:
		case RoundedLine: break;

		case Bezier:
		case FillBezier: {
			m_drawLayer->drawDot(m_begin, 7, { 1, 0.5, 1, 1 });
			m_drawLayer->drawDot(m_end, 5, { 0.5, 1, 1, 1 });
			m_drawLayer->drawSegment(m_begin, m_end, 1, { 0.5, 1, 1, 1 });
			if (this->isContinuing()) {
				auto control = m_begin - (m_end - m_begin);
				m_drawLayer->drawDot(control, 5, { 0.5, 1, 1, 1 });
				m_drawLayer->drawSegment(m_begin, control, 1, { 0.5, 1, 1, 1 });

				m_drawLayer->drawDot(m_lastBegin, 7, { 1, 0.5, 1, 1 });
				m_drawLayer->drawDot(m_lastEnd, 5, { 0.5, 1, 1, 1 });
				m_drawLayer->drawSegment(m_lastBegin, m_lastEnd, 1, { 0.5, 1, 1, 1 });
			}
		} break;

		default: break;
	}
}

void LineDrawer::generateLine() {
	this->clearObjects();

	if (this->shouldDraw()) {
		auto generated = this->generate();

		// auto generated = LineGenerator().generate(m_fields->begin, m_fields->end, { 15 });

		for (auto& data : generated) {
			// log::debug("object {} {}", data.position, data.rotation);
			auto obj = GameObject::createWithKey(data.id);

			obj->customSetup();
			obj->addColorSprite();
			obj->setupCustomSprites();

			obj->setPosition(data.position);
			obj->setStartPos(data.position);
			m_lineObjects->addObject(obj);

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

			m_lineLayer->addChild(obj);
		}
	}
}

void LineDrawer::began(cocos2d::CCPoint pos) {
	m_begin = pos;
}

void LineDrawer::moved(cocos2d::CCPoint pos) {
	m_end = pos;
	this->generateLine();
	this->drawOverlay();
}

void LineDrawer::ended(cocos2d::CCPoint pos) {
	m_end = pos;
	this->generateLine();

	for (auto obj : CCArrayExt<GameObject*>(m_lineObjects)) {
		m_editor->m_editorLayer->addToSection(obj);
		m_editor->m_editorLayer->addSpecial(obj);
	}
	this->clearObjects();

	this->setLastPoints();
	m_dragCount++;
}

void LineDrawer::reset() {
	m_dragCount = 0;
	m_drawLayer->clear();
}

LineDrawer::LineDrawer() {
	m_lineObjects = CCArray::create();
	m_lineLayer = CCLayer::create();
}

void LineDrawer::setEditor(EditorUI* editor) {
	m_editor = editor;
	m_drawLayer = CCDrawNode::create();
	m_editor->m_editorLayer->m_objectLayer->addChild(m_lineLayer);
	m_editor->m_editorLayer->m_objectLayer->addChild(m_drawLayer, 100);
}