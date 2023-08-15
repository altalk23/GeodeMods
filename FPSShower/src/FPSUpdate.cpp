#include <FPSUpdate.hpp>
#include <Geode/Loader.hpp>

using namespace geode::prelude;
using namespace tulip::fps;

FPSUpdate* FPSUpdate::get() {
	static FPSUpdate* s_sharedState = nullptr;
	if (!s_sharedState) {
		s_sharedState = new (std::nothrow) FPSUpdate;
		s_sharedState->init();
	}
	return s_sharedState;
}

bool FPSUpdate::init() {
	return true;
}

void FPSUpdate::setOverlay(FPSOverlay* overlay) {
	if (m_overlay) {
		this->releaseOverlay();
	}
	m_overlay = overlay;
	m_overlay->setFramerate(m_framerate);
	SceneManager::get()->keepAcrossScenes(m_overlay);
}

void FPSUpdate::releaseOverlay() {
	if (m_overlay) {
		SceneManager::get()->forget(m_overlay);
		m_overlay = nullptr;
	}
}

double FPSUpdate::getFramerate() {
	return m_framerate;
}

void FPSUpdate::update(float dt) {
	++m_frames;
	m_accumulate += dt;

	if (m_accumulate > m_resetInterval) {
		m_framerate = m_frames / m_accumulate;
		m_framerate = std::round(m_framerate * 10) / 10;
		m_frames = 0;
		m_accumulate = 0;

		if (m_overlay) {
			m_overlay->setFramerate(m_framerate);
		}
	}
}