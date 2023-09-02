#include <LineGenerator.hpp>

using namespace geode::prelude;
using namespace tulip::editor;
#include <array>
#include <numbers>

float LineGenerator::getRotation(CCPoint const& begin, CCPoint const& end) {
	auto rotation = -(end - begin).getAngle();

	if (CCKeyboardDispatcher::get()->getShiftKeyPressed()) {
		rotation = std::round(rotation / std::numbers::pi * 2) * std::numbers::pi / 2;
	}

	return rotation;
}

CCPoint LineGenerator::getBeginpoint(CCPoint const& begin, CCPoint const& end) {
	auto beginpoint = begin;

	if (CCKeyboardDispatcher::get()->getAltKeyPressed()) {
		beginpoint =
			CCPointMake(std::round(beginpoint.x / 30) * 30, std::round(beginpoint.y / 30) * 30);
	}

	return beginpoint;
}

CCPoint LineGenerator::getEndpoint(CCPoint const& begin, CCPoint const& end) {
	auto endpoint = end;

	if (CCKeyboardDispatcher::get()->getShiftKeyPressed()) {
		if (std::abs(end.x - begin.x) > std::abs(end.y - begin.y)) {
			endpoint = CCPoint { end.x, begin.y };
		}
		else {
			endpoint = CCPoint { begin.x, end.y };
		}
	}

	if (CCKeyboardDispatcher::get()->getAltKeyPressed()) {
		endpoint = CCPointMake(std::round(endpoint.x / 30) * 30, std::round(endpoint.y / 30) * 30);
	}

	return endpoint;
}

std::vector<ObjectData> LineGenerator::generate(CCPoint begin, CCPoint end, LineData const& data) {
	auto ret = std::vector<ObjectData>();

	begin = this->getBeginpoint(begin, end);
	end = this->getEndpoint(begin, end);

	auto length = begin.getDistance(end);
	auto width = data.thickness;
	if (width > length) {
		return ret;
		std::swap(length, width);
	}

	auto units = length / width;

	this->addUnits(ret, units, width);

	this->rotate(ret, this->getRotation(begin, end));
	this->move(ret, begin);

	return ret;
}

void LineGenerator::addUnits(std::vector<ObjectData>& objects, float units, float width) {
	struct Value {
		float size;
		float scale;
		CCPoint offset;
		int id;
	};

	static std::array<Value, 8> things = {
		Value { 1, 30, { 0, 0 }, 211 },      Value { 1.5, 10, { 0, 0 }, 580 },
		Value { 3, 10, { 0, 0 }, 579 },      Value { 4, 7.5, { 0, 11.25 }, 1191 },
		Value { 10, 1.5, { 0, 4.25 }, 508 }, Value { 15, 1, { 0, 0 }, 1757 },
		Value { 20, 1.5, { 0, 4.25 }, 507 }, Value { 30, 1, { 0, 0 }, 1753 }
	};

	auto selected = std::upper_bound(
		things.begin(), things.end(), units,
		[](auto const& unit, auto const& val) {
			return unit < val.size;
		}
	);

	if (selected == things.begin()) {
		return;
	}
	--selected;

	int count = std::ceil(units / selected->size);

	for (int i = 0; i < count; ++i) {
		auto scale = width / selected->scale;
		auto pos = CCPointMake(width * (i + 0.5) * selected->size, 0) - selected->offset * scale;
		auto obj = ObjectData { pos, width / selected->scale, 0, selected->id };
		objects.push_back(obj);
	}

	objects.back().position.x = units * width - width * selected->size * 0.5;
}

void LineGenerator::rotate(std::vector<ObjectData>& objects, float rotation) {
	for (auto& obj : objects) {
		obj.position = obj.position.rotateByAngle({ 0, 0 }, -rotation);
		obj.rotation = rotation / std::numbers::pi * 180;
	}
}

void LineGenerator::move(std::vector<ObjectData>& objects, cocos2d::CCPoint const& pos) {
	for (auto& obj : objects) {
		obj.position += pos;
	}
}

std::vector<ObjectData> RoundedLineGenerator::generate(
	CCPoint begin, CCPoint end, LineData const& data
) {
	auto ret = LineGenerator::generate(begin, end, data);

	begin = this->getBeginpoint(begin, end);
	end = this->getEndpoint(begin, end);

	ret.push_back({ begin, data.thickness / 9, 0, 725 });
	ret.push_back({ end, data.thickness / 9, 0, 725 });

	return ret;
}