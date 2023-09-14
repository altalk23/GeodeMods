#import <Cocoa/Cocoa.h>
#include <Geode/Bindings.hpp>
#include <Geode/DefaultInclude.hpp>
#include <Geode/Loader.hpp>
#import <Geode/cocos/platform/mac/EAGLView.h>
#include <Geode/utils/ObjcHook.hpp>
#import <objc/runtime.h>

using namespace geode::prelude;

// TODO:
namespace tulip::fps {
	template <class Lambda>
	inline void openGLWrapped(Lambda&& func) {
		if (!geode::Loader::get()->isModLoaded("alk.fps-bypass")) {
			return func();
		}

		NSOpenGLView* openGLView = [NSClassFromString(@"EAGLView") sharedEGLView];
		NSOpenGLContext* glContext = [openGLView openGLContext];

		CGLLockContext([glContext CGLContextObj]);

		func();

		CGLUnlockContext([glContext CGLContextObj]);
	}
}

using namespace tulip::fps;

#include <Geode/modify/PlatformToolbox.hpp>

struct ToggleFullscreenReplace : Modify<ToggleFullscreenReplace, PlatformToolbox> {
	static void toggleFullScreen(bool fullscreen) {
		auto window = [[NSApplication sharedApplication] mainWindow];

		[window toggleFullScreen:nil];
		GameManager::get()->setGameVariable("0025", !fullscreen);
	}
};

static IMP s_applicationDidFinishLaunching;

void applicationDidFinishLaunching(id self, SEL selector, NSNotification* notification) {
	auto windowed = GameManager::get()->getGameVariable("0025");
	log::debug("windowed {}", windowed);
	if (!windowed) {
		GameManager::get()->setGameVariable("0025", true);
	}
	// using Type = decltype(&applicationDidFinishLaunching);
	// reinterpret_cast<Type>(s_applicationDidFinishLaunching)(self, selector, notification);

	[self applicationDidFinishLaunching:notification];

	auto window = *(NSWindow**)((uintptr_t)self + 0x8);

	[window setBackgroundColor:NSColor.whiteColor];
	[window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

	if (!windowed) {
		PlatformToolbox::toggleFullScreen(true);
	}
}

void windowWillEnterFullScreen(id self, SEL selector, NSNotification* notification) {
	log::debug("windowWillEnterFullScreen");
	auto screenFrame = [[NSScreen mainScreen] frame];

	CCEGLView::sharedOpenGLView()->setFrameSize(screenFrame.size.width, screenFrame.size.height);
	CCDirector::get()->updateScreenScale({ (float)screenFrame.size.width,
										   (float)screenFrame.size.height });

	[[NSClassFromString(@"EAGLView") sharedEGLView] setNeedsDisplay:YES];

	GameManager::get()->setGameVariable("0025", false);
}

void windowWillExitFullScreen(id self, SEL selector, NSNotification* notification) {
	log::debug("windowWillExitFullScreen");
	auto screenFrame = [[NSScreen mainScreen] frame];

	CCEGLView::sharedOpenGLView()->setFrameSize(screenFrame.size.width, screenFrame.size.height);
	CCDirector::get()->updateScreenScale({ (float)screenFrame.size.width,
										   (float)screenFrame.size.height });

	[[NSClassFromString(@"EAGLView") sharedEGLView] setNeedsDisplay:YES];

	GameManager::get()->setGameVariable("0025", true);
}

void applicationWillBecomeActive(id self, SEL selector, NSNotification* notification) {
	openGLWrapped([]() {
		auto windowed = GameManager::get()->getGameVariable("0025");
		if (windowed) {
			AppDelegate::get()->applicationWillEnterForeground();
		}
		else {
			AppDelegate::get()->applicationWillBecomeActive();
		}
	});
}

void applicationWillResignActive(id self, SEL selector, NSNotification* notification) {
	openGLWrapped([]() {
		auto windowed = GameManager::get()->getGameVariable("0025");
		if (windowed) {
			AppDelegate::get()->applicationDidEnterBackground();
		}
		else {
			AppDelegate::get()->applicationWillResignActive();
		}
	});
}

template <class Type>
IMP replaceMethod(Class class_, SEL selector, Type function) {
	return class_replaceMethod(class_, selector, (IMP)function, @encode(Type));
}

template <class Type>
bool addMethod(Class class_, SEL selector, Type function) {
	return class_addMethod(class_, selector, (IMP)function, @encode(Type));
}

template <auto Function>
void empty() {}

template <auto Function>
void createHook(std::string const& className, std::string const& funcName) {
	if (auto res = ObjcHook::create(className, funcName, Function, &empty<Function>)) {
		(void)Mod::get()->addHook(res.unwrap());
	}
}

void appControllerHooks() {
	// replaceMethod(class_, @selector(applicationWillBecomeActive:), &applicationWillBecomeActive);
	// replaceMethod(class_, @selector(applicationWillResignActive:), &applicationWillResignActive);
}

$execute {
	createHook<&windowWillEnterFullScreen>("AppController", "windowWillEnterFullScreen:");
	createHook<&windowWillExitFullScreen>("AppController", "windowWillExitFullScreen:");
	createHook<&applicationDidFinishLaunching>("AppController", "applicationDidFinishLaunching:");
}

// id initWithFrame(EAGLView* self, SEL selector, NSRect frameRect, NSOpenGLPixelFormat* format) {
// 	NSOpenGLPixelFormatAttribute attribs[] = {
// 		// NSOpenGLPFAAccelerated,
// 		// NSOpenGLPFANoRecovery,
// 		NSOpenGLPFADoubleBuffer,  NSOpenGLPFADepthSize,         24, NSOpenGLPFAStencilSize, 8,
// 		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy, 0
// 		// , 0
// 	};

// 	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];

// 	auto ret = [self initWithFrame:frameRect pixelFormat:pixelFormat];

// 	// log::debug("OpenGL version = {}", (char const*)glGetString(GL_VERSION));
// 	// log::debug("GLSL version = {}", (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION));

// 	return ret;
// }

// $execute {
// 	if (auto res = ObjcHook::create("EAGLView", "initWithFrame:pixelFormat:", &initWithFrame)) {
// 		Mod::get()->addHook(res.unwrap());
// 	}
// }

// #include <Geode/modify/CCGLProgram.hpp>

// class MyShaderProgram : public CCGLProgram {
// public:
// };

// struct CompileShader : Modify<CompileShader, CCGLProgram> {
// 	std::string getShaderLog(GLuint object) {
// 		GLint length, written;
// 		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
// 		log::debug("object {} log length {}", object, length);
// 		if (length <= 0) {
// 			return "";
// 		}
// 		auto stuff = new char[length + 1];
// 		glGetShaderInfoLog(object, length, &written, stuff);
// 		std::string result(stuff);
// 		log::debug(
// 			"{} {} {} {}", (void*)stuff[0], (void*)stuff[1], (void*)stuff[2], (void*)stuff[3]
// 		);
// 		delete[] stuff;
// 		return result;
// 	}

// 	bool compileShader(GLuint* shader, GLenum type, GLchar const* source) {
// 		GLint status;

// 		if (!source) {
// 			return false;
// 		}

// 		GLchar const* sources[] = {
// 			"#version 120\n"
// 			"uniform mat4 CC_PMatrix;\n"
// 			"uniform mat4 CC_MVMatrix;\n"
// 			"uniform mat4 CC_MVPMatrix;\n"
// 			"uniform vec4 CC_Time;\n"
// 			"uniform vec4 CC_SinTime;\n"
// 			"uniform vec4 CC_CosTime;\n"
// 			"uniform vec4 CC_Random01;\n"
// 			"//CC INCLUDES END\n\n",
// 			source,
// 		};

// 		*shader = glCreateShader(type);
// 		glShaderSource(*shader, sizeof(sources) / sizeof(*sources), sources, NULL);
// 		glCompileShader(*shader);

// 		log::debug("compiling {}, {}", *shader, type);

// 		glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

// 		log::debug("OpenGL extensions = {}", (char const*)glGetString(GL_EXTENSIONS));
// 		if (!status) {
// 			GLsizei length;
// 			glGetShaderiv(*shader, GL_SHADER_SOURCE_LENGTH, &length);
// 			GLchar* src = (GLchar*)malloc(sizeof(GLchar) * length);

// 			glGetShaderSource(*shader, length, NULL, src);

// 			log::debug("GLSL version = {}", (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION));

// 			log::error("cocos2d: ERROR: Failed to compile shader:\n{}", src);
// 			log::debug("cocos2d: {}", getShaderLog(*shader));
// 			free(src);

// 			return false;
// 		}
// 		return (status == GL_TRUE);
// 	}
// };
