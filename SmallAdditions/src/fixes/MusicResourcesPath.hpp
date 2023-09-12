
#ifdef GEODE_IS_MACOS

#include <Geode/modify/MusicDownloadManager.hpp>

struct MusicResourcesPath : Modify<MusicResourcesPath, MusicDownloadManager> {
	static void onModify(auto& self) {
		(void)self.setHookPriority("MusicResourcesPath::pathForSong", 0x10000); // not calling orig
	}

	gd::string pathForSong(int songID) {
		std::stringstream stream;
		stream << songID << ".mp3";
		auto savePath =
			ghc::filesystem::path(CCFileUtils::sharedFileUtils()->getWritablePath().c_str());
		auto path = savePath / "Resources" / stream.str();
		return gd::string(path.string());
	}
};

$execute {
	auto savePath =
		ghc::filesystem::path(CCFileUtils::sharedFileUtils()->getWritablePath().c_str());
	ghc::filesystem::create_directory(savePath / "Resources");
}

#endif
