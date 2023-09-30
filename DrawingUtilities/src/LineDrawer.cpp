#include <LineDrawer.hpp>
#include <LineGenerator.hpp>
#include <nanosvg.h>

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
		case RoughBezier:
		case Bezier: return m_dragCount > 0;
		default: return true;
	}
}

bool LineDrawer::isContinuing() {
	using enum UsedGenerator;
	switch (m_used) {
		case RoughBezier:
		case Bezier: return m_dragCount > 1;
		default: return false;
	}
}

void LineDrawer::setLastPoints() {
	using enum UsedGenerator;
	switch (m_used) {
		case RoughBezier:
		case Bezier: {
			m_lastBegin = m_begin;
			m_lastEnd = m_end;
			// if (this->isContinuing()) {
			// 	m_lastEnd = m_begin - (m_end - m_begin);
			// }
		} break;

		default: break;
	}
}

void LineDrawer::svgTest() {
	struct NSVGimage* image;
	auto path = file::pickFile(file::PickMode::OpenFile, { std::nullopt, {} });
	if (!path) {
		return;
	}
	image = nsvgParseFromFile(path->string().c_str(), "px", 96);
	// log::debug("size: {} x {}\n", image->width, image->height);

	auto objCount = 0;
	// Use...
	for (auto shape = image->shapes; shape != NULL; shape = shape->next) {
		float thickness = shape->strokeWidth;
		// log::debug("path thickness: {}", thickness);
		for (auto path = shape->paths; path != NULL; path = path->next) {
			// log::debug("point count: {}", path->npts);
			for (auto i = 0; i < path->npts - 1; i += 3) {
				float* p = &path->pts[i * 2];
				auto p1 = CCPointMake(p[0] + 300, image->height - p[1]);
				auto p2 = CCPointMake(p[2] + 300, image->height - p[3]);
				auto p3 = CCPointMake(p[4] + 300, image->height - p[5]);
				auto p4 = CCPointMake(p[6] + 300, image->height - p[7]);

				// log::debug("points ({}) ({}) ({}) ({})", p1, p2, p3, p4);

				auto generated =
					BezierLineGenerator().generate({ p1, p2, p3, p4 }, { thickness, i != 0, 0.2 });
				// log::debug("bezier generated {}", generated.size());
				if (generated.size() < 150) {
					objCount += generated.size();

					// log::debug("obj count {}, total {}", generated.size(), objCount);
					++m_linkID;
					this->drawData(generated);
				}
			}
		}
	}

	for (auto obj : CCArrayExt<GameObject*>(m_lineObjects)) {
		m_editor->m_editorLayer->addToSection(obj);
		m_editor->m_editorLayer->addSpecial(obj);
	}
	this->clearObjects();

	// Delete
	nsvgDelete(image);
}

std::vector<ObjectData> LineDrawer::generate() {
	using enum UsedGenerator;
	std::unique_ptr<LineGenerator> generator;
	std::vector<CCPoint> points;

	float bezierDetail = 0.1;

	switch (m_used) {
		case Line: {
			generator = std::make_unique<LineGenerator>();
			points = { m_begin, m_end };
		} break;

		case RoundedLine: {
			generator = std::make_unique<RoundedLineGenerator>();
			points = { m_begin, m_end };
		} break;

		case RoughBezier: {
			bezierDetail = 0.04;
			[[fallthrough]];
		}
		case Bezier: {
			generator = std::make_unique<BezierLineGenerator>();
			auto control = this->shouldDraw() ? m_begin - (m_end - m_begin) : m_end;
			points = { m_lastBegin, m_lastEnd, control, m_begin };
		} break;

		default: return {};
	}

	return generator->generate(points, { m_thickness, this->isContinuing(), bezierDetail });
}

void LineDrawer::drawOverlay() {
	using enum UsedGenerator;
	m_drawLayer->clear();

	switch (m_used) {
		case Line:
		case RoundedLine: break;

		case RoughBezier:
		case Bezier: {
			m_drawLayer->drawDot(m_begin, 7, { 1, 0.5, 1, 1 });
			if (this->shouldDraw()) {
				auto control = m_begin - (m_end - m_begin);
				m_drawLayer->drawDot(control, 5, { 0.5, 1, 1, 1 });
				m_drawLayer->drawSegment(m_begin, control, 1, { 0.5, 1, 1, 1 });

				m_drawLayer->drawDot(m_end, 5, { 1, 1, 0.5, 1 });
				m_drawLayer->drawSegment(m_begin, m_end, 1, { 1, 1, 0.5, 1 });

				m_drawLayer->drawDot(m_lastBegin, 7, { 1, 0.5, 1, 1 });
				m_drawLayer->drawDot(m_lastEnd, 5, { 0.5, 1, 1, 1 });
				m_drawLayer->drawSegment(m_lastBegin, m_lastEnd, 1, { 0.5, 1, 1, 1 });
			}
			else {
				m_drawLayer->drawDot(m_end, 5, { 0.5, 1, 1, 1 });
				m_drawLayer->drawSegment(m_begin, m_end, 1, { 0.5, 1, 1, 1 });
			}
		} break;

		default: break;
	}
}

void LineDrawer::drawData(std::vector<ObjectData> const& generated) {
	for (auto& data : generated) {
		// log::debug("object {} {}", data.position, data.rotation);
		auto obj = GameObject::createWithKey(data.id);

		obj->customSetup();
		obj->addColorSprite();
		obj->setupCustomSprites();

		obj->setPosition(data.position);
		obj->setStartPos(data.position);
		m_lineObjects->addObject(obj);

		obj->setRotation(std::round(data.rotation));

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

		// obj->m_linkedGroup = m_linkID;

		m_lineLayer->addChild(obj);
	}
}

void LineDrawer::generateLine() {
	this->clearObjects();

	if (this->shouldDraw()) {
		auto generated = this->generate();

		// auto generated = LineGenerator().generate(m_fields->begin, m_fields->end, { 15 });

		this->drawData(generated);
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
	++m_linkID;

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

	// auto test = CCSprite::create("test.png"_spr);
	// test->setOpacity(63);
	// test->setPosition({ 300, 300 });
	// m_editor->m_editorLayer->m_objectLayer->addChild(test);
}