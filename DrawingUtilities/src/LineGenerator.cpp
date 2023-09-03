#include "Agg.hpp"

#include <LineGenerator.hpp>

using namespace geode::prelude;
using namespace tulip::editor;
#include <array>
#include <numbers>

float LineGenerator::getRotation(CCPoint const& begin, CCPoint const& end) {
	auto rotation = -(end - begin).getAngle();

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
		std::swap(length, width);
		auto mid = begin.lerp(end, 0.5);
		auto out = CCPoint::forAngle((end - begin).getAngle() + std::numbers::pi / 2) * length / 2;
		begin = mid + out;
		end = mid - out;
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
		Value { 1, 30, { 0, 0 }, 211 }, Value { 1.5, 10, { 0, 0 }, 580 },
		Value { 3, 10, { 0, 0 }, 579 }, Value { 4, 7.5, { 0, 11.25 }, 1191 },
		Value { 10, 1.5, { 0, 4.25 }, 508 },
		/*Value { 120.0 / 17.0, 17.0 / 4.0, { 0, 12.125 }, 1277 },*/
		Value { 15, 1, { 0, 0 }, 1757 }, Value { 20, 1.5, { 0, 4.25 }, 507 },
		Value { 30, 1, { 0, 0 }, 1753 }
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

	if (objects.size() > 0) {
		// this should never be 0 in normal circumstances but when two points are on top of each
		// other they are so
		objects.back().position.x = units * width - width * selected->size * 0.5;
	}
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

std::vector<ObjectData> BezierLineGenerator::generate(
	cocos2d::CCPoint ap, cocos2d::CCPoint bp, cocos2d::CCPoint cp, cocos2d::CCPoint dp,
	LineData const& data
) {
	auto generator = agg::curve4_div(ap, bp, cp, dp);

	auto ret = std::vector<ObjectData>();

	for (size_t i = 0; i < generator.m_points.size() - 1; ++i) {
		auto add = LineGenerator().generate(generator.m_points[i], generator.m_points[i + 1], data);
		ret.insert(ret.end(), add.begin(), add.end());
	}

	return ret;
}

// time for some hardcore math
// what did i do to deserve this
namespace {
	struct BezierFill {
		static auto a(auto p1, auto p2) {
			return p1.y - p2.y;
		}

		static auto b(auto p1, auto p2) {
			return p2.x - p1.x;
		}

		static auto c(auto p1, auto p2) {
			return (p1.x - p2.x) * p1.y + (p2.y - p1.y) * p1.x;
		}

		static auto intersect(auto p1, auto p2, auto r1, auto r2) {
			auto a1 = a(p1, p2);
			auto b1 = b(p1, p2);
			auto c1 = c(p1, p2);

			auto a2 = a(r1, r2);
			auto b2 = b(r1, r2);
			auto c2 = c(r1, r2);
			return CCPointMake(
				(b1 * c2 - b2 * c1) / (a1 * b2 - a2 * b1), (c1 * a2 - c2 * a1) / (a1 * b2 - a2 * b1)
			);
		}

		static auto angle(auto p1, auto p2, auto p3) {
			return (p1 - p2).getAngle(p3 - p2);
		}

		static auto offset(auto p1, auto p2, auto p3, auto width) {
			if (angle(p1, p2, p3) < 0) {
				// right side
				return ((p1 - p2).getPerp().normalize() * width / 2);
			}
			else {
				// left side
				return ((p1 - p2).getRPerp().normalize() * width / 2);
			}
		}

		static auto extend(auto p1, auto p2, auto p3, auto width) {
			auto poff = offset(p1, p2, p3, width);
			auto roff = offset(p3, p2, p1, width);

			auto s1 = p1 + poff;
			auto s2 = p2 + poff;
			auto r1 = p2 + roff;
			auto r2 = p3 + roff;

			auto inter = intersect(s1, s2, r1, r2);

			return std::make_pair(inter - roff, inter - poff);
		}
	};
}

std::vector<ObjectData> FillBezierLineGenerator::generate(
	cocos2d::CCPoint ap, cocos2d::CCPoint bp, cocos2d::CCPoint cp, cocos2d::CCPoint dp,
	LineData const& data
) {
	auto generator = agg::curve4_div(ap, bp, cp, dp);

	auto ret = std::vector<ObjectData>();
	auto width = data.thickness;

	for (size_t i = 0; i < generator.m_points.size() - 1; ++i) {
		auto begin = generator.m_points[i];
		auto end = generator.m_points[i + 1];

		if (i > 0) {
			auto a1 = generator.m_points[i - 1];
			auto a2 = generator.m_points[i];
			auto a3 = generator.m_points[i + 1];

			auto extend = BezierFill::extend(a1, a2, a3, width).first;

			if (extend.getDistance(begin) < width / 2) {
				begin = extend;
			}
		}

		if (i < generator.m_points.size() - 2) {
			auto a2 = generator.m_points[i];
			auto a3 = generator.m_points[i + 1];
			auto a4 = generator.m_points[i + 2];

			auto extend = end = BezierFill::extend(a2, a3, a4, width).second;

			if (extend.getDistance(begin) < width / 2) {
				end = extend;
			}
			else {
				ret.push_back({ end, data.thickness / 9, 0, 725 });
			}
		}

		auto add = LineGenerator().generate(begin, end, data);
		ret.insert(ret.end(), add.begin(), add.end());
	}

	return ret;
}
