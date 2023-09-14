#pragma once

#include "LineGenerator.hpp"

#include <Geode/Loader.hpp>

namespace tulip::editor {
	enum class UsedGenerator {
		Line,
		RoundedLine,
		Bezier,
		RoughBezier,
	};

	class LineDrawer {
	public:
		EditorUI* m_editor;
		geode::Ref<cocos2d::CCArray> m_lineObjects;
		geode::Ref<cocos2d::CCLayer> m_lineLayer;
		geode::Ref<cocos2d::CCDrawNode> m_drawLayer;

		UsedGenerator m_used;
		float m_thickness = 10;

		cocos2d::CCPoint m_lastBegin;
		cocos2d::CCPoint m_lastEnd;
		cocos2d::CCPoint m_begin;
		cocos2d::CCPoint m_end;

		LineDrawer();

		int m_dragCount = 0;

		void drawOverlay();

		void setEditor(EditorUI* editor);
		void clearObjects();
		void generateLine();

		bool shouldDraw();
		bool isContinuing();

		void setLastPoints();

		std::vector<ObjectData> generate();

		void began(cocos2d::CCPoint pos);
		void moved(cocos2d::CCPoint pos);
		void ended(cocos2d::CCPoint pos);

		void reset();
	};
}