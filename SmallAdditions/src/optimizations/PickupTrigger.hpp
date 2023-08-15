
#include <Geode/modify/GJEffectManager.hpp>

struct CountTriggerChange : Modify<CountTriggerChange, GJEffectManager> {
	void countChangedForItem(int item) {
		auto gameLayer = GJBaseGameLayer::get();
		int value = m_itemValues[item];

		auto actions = static_cast<CCArray*>(m_countActions->objectForKey(item));
		if (actions) {
			for (auto action : CCArrayExt<CountTriggerAction*>(actions)) {
				if ((action->m_previousCount < action->m_targetCount &&
					 value >= action->m_targetCount) ||
					(action->m_previousCount > action->m_targetCount &&
					 value <= action->m_targetCount
					)) { // second one is to be consistent with rob's system, should be removed imo

					if (gameLayer) {
						gameLayer->toggleGroupTriggered(
							action->m_targetID, action->m_activateGroup
						);
					}
					else {
						toggleGroup(action->m_targetID, action->m_activateGroup);
					}

					if (!action->m_multiActivate) {
						actions->fastRemoveObject(action);
						continue;
					}
				}
				action->m_previousCount = value;
			}
		}
	}
};

#include <Geode/modify/GJBaseGameLayer.hpp>

struct PickupTriggerChange : Modify<PickupTriggerChange, GJBaseGameLayer> {
	void collectItem(int item, int count) {
		if (item < 0) {
			item = 0;
		}
		if (item > 1100) {
			item = 1100;
		}

		m_effectManager->m_itemValues[item] += count;
		m_effectManager->countChangedForItem(item);
		this->updateCounters(item, m_effectManager->m_itemValues[item]);
	}
};