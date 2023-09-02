#include <Geode/Loader.hpp>

namespace tulip::editor {
	struct LineData {
		float thickness;
	};

	struct ObjectData {
		cocos2d::CCPoint position;
		float scale;
		float rotation;
		int id;
	};

	class LineGenerator {
	protected:
		void addUnits(std::vector<ObjectData>& objects, float units, float width);

		void rotate(std::vector<ObjectData>& objects, float rotation);
		void move(std::vector<ObjectData>& objects, cocos2d::CCPoint const& pos);

	public:
		float getRotation(cocos2d::CCPoint const& begin, cocos2d::CCPoint const& end);
		cocos2d::CCPoint getBeginpoint(cocos2d::CCPoint const& begin, cocos2d::CCPoint const& end);
		cocos2d::CCPoint getEndpoint(cocos2d::CCPoint const& begin, cocos2d::CCPoint const& end);

		virtual std::vector<ObjectData> generate(
			cocos2d::CCPoint begin, cocos2d::CCPoint end, LineData const& data
		);
	};

	class RoundedLineGenerator : public LineGenerator {
	public:
		std::vector<ObjectData> generate(
			cocos2d::CCPoint begin, cocos2d::CCPoint end, LineData const& data
		) override;
	};
}