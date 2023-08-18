
#include <Geode/modify/CCMotionStreak.hpp>

struct MotionStreakCutoff : Modify<MotionStreakCutoff, CCMotionStreak> {
	bool initWithFade(
		float fade, float minSeg, float stroke, ccColor3B const& color, CCTexture2D* texture
	) {
		if (!CCMotionStreak::initWithFade(fade, minSeg, stroke, color, texture)) {
			return false;
		}

		auto interval = CCDirector::sharedDirector()->getAnimationInterval();

		log::debug("interval {}", interval);

		m_uMaxPoints = (int)(fade / interval) + 2;
		m_uNuPoints = 0;
		free(m_pPointState);
		free(m_pPointVertexes);
		m_pPointState = (float*)malloc(sizeof(float) * m_uMaxPoints);
		m_pPointVertexes = (CCPoint*)malloc(sizeof(CCPoint) * m_uMaxPoints);

		free(m_pVertices);
		free(m_pTexCoords);
		free(m_pColorPointer);
		m_pVertices = (ccVertex2F*)malloc(sizeof(ccVertex2F) * m_uMaxPoints * 2);
		m_pTexCoords = (ccTex2F*)malloc(sizeof(ccTex2F) * m_uMaxPoints * 2);
		m_pColorPointer = (GLubyte*)malloc(sizeof(GLubyte) * m_uMaxPoints * 2 * 4);

		return true;
	}
};