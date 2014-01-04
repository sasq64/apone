#include "../window.h"
#include "../tween.h"
#include <stdio.h>
#include <unordered_map>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <cstdlib>
#include <chrono>
#include <native_app_glue/android_native_app_glue.h>

#include <android/log.h>

//#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Grappix", __VA_ARGS__))
//#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "Grappix", __VA_ARGS__))



using namespace std;

extern int main(int argc, char **argv);


struct TouchEvent {
	TouchEvent(int w = 0, int i = 0, float xx = 0, float yy = 0) : what(w), id(i), x(xx), y(yy) {}
	enum {
		UP,
		DOWN
	};
	int what, id;
	float x,y;
};

struct KeyEvent {
	KeyEvent(int w = 0, int c = 0) : what(w), code(c) {}
	enum {
		UP,
		DOWN
	};
	int what, code;
};

class AndroidHost {
public:
	AndroidHost() : app(nullptr), nativeWindow(nullptr), focus(true) {}

	int32_t handleAndroidEvent(AInputEvent *event) {
		int type = AInputEvent_getType(event);
		if(type == AINPUT_EVENT_TYPE_KEY) {
			int code = AKeyEvent_getKeyCode(event);
			int action = AKeyEvent_getAction(event);
			if(action == AKEY_EVENT_ACTION_DOWN) {
				//LOGD("Pushed %d", code);
				keyEvents.push_back(KeyEvent(KeyEvent::DOWN, code));
			}
		} else if (type == AINPUT_EVENT_TYPE_MOTION) {

			//engine->animating = 1;
			//AInputEvent_getType();

			int n = AMotionEvent_getPointerCount(event);
			int action = AMotionEvent_getAction(event);

			int index = action >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			int what = action & AMOTION_EVENT_ACTION_MASK;

			
			int id = AMotionEvent_getPointerId(event, index);
			int x = AMotionEvent_getX(event, id);
			int y = AMotionEvent_getY(event, id);

			if(what == 6 || what == 1) {
				//LOGI("%d UP", id);
				touchEvents.push_back(TouchEvent(TouchEvent::UP, id, x, y));
			} else if(what == 0 || what == 5) {
				//LOGI("%d DOWN", id);
				touchEvents.push_back(TouchEvent(TouchEvent::DOWN, id, x, y));
			} else if(what == 2) {
				//what = TouchEvent::MOVE;
				//LOGI("%d MOVE: %d %d", index, x, y);
				for(int i=0; i<n; i++) {
					int x = AMotionEvent_getX(event, i);
					int y = AMotionEvent_getY(event, i);
					int id = AMotionEvent_getPointerId(event, i);

					int xy = x | y<<16;

					//if(pos[id] != xy) {
					//	pos[id] = xy;
						//LOGI("MOVE %d (%d): %d %d\n", i, id, x, y);
					//	touchEvents.push(TouchEvent(what, id, x, y));
					//}

				}

			}
	/*
			for(int i=0; i<n; i++) {

				int x = AMotionEvent_getX(event, i);
				int y = AMotionEvent_getY(event, i);
				int id = AMotionEvent_getPointerId(event, i);
				LOGI("%d: %d %d\n", id, x, y);
				host->addMotionEvent(AndroidHostSystem::MotionEvent(id, x, y));
			} */
			return 1;
		}
		return 0;
	}

	int initScreen(ANativeWindow *nativeWindow) {

		/*
		 * Here specify the attributes of the desired configuration.
		 * Below, we select an EGLConfig with at least 8 bits per color
		 * component compatible with on-screen windows
		 */
		const EGLint attribs[] = {
				//EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_BLUE_SIZE, 8,
				EGL_GREEN_SIZE, 8,
				EGL_RED_SIZE, 8,
				EGL_NONE
		};

		EGLint w, h, dummy, format;
		EGLint numConfigs;
		EGLConfig config;
		EGLConfig configList[32];
		EGLSurface surface;
		EGLContext context;

		EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

		EGLint m0;
		EGLint m1;

		eglInitialize(display, &m0, &m1);

		LOGI("EGL v%d.%d", m0, m1);	

		/* Here, the application chooses the configuration it desires. In this
		 * sample, we have a very simplified selection process, where we pick
		 * the first EGLConfig that matches our criteria */
		eglGetConfigs(display, configList, 32, &numConfigs);

		LOGI("Found %d matching configs", numConfigs);

		for(int i=0; i<numConfigs; i++) {
			EGLint conf, id, stype, redSize, caveat, sbuffers;
			eglGetConfigAttrib(display, configList[i], EGL_CONFORMANT, &conf);
			eglGetConfigAttrib(display, configList[i], EGL_CONFIG_ID, &id);
			eglGetConfigAttrib(display, configList[i], EGL_SURFACE_TYPE, &stype);
			eglGetConfigAttrib(display, configList[i], EGL_RED_SIZE, &redSize);
			eglGetConfigAttrib(display, configList[i], EGL_CONFIG_CAVEAT, &caveat);
			eglGetConfigAttrib(display, configList[i], EGL_SAMPLE_BUFFERS, &sbuffers);
			
			
			LOGI("Config %d (%d) conformant %x RED %d caveat %x stype %x", i, id, conf, redSize, caveat, stype);

			if((conf & EGL_OPENGL_ES2_BIT) && (stype & EGL_WINDOW_BIT)) {
				config = configList[i];
				if(sbuffers > 0) {
					break;
				}
			}
		}

		/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
		 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
		 * As soon as we picked a EGLConfig, we can safely reconfigure the
		 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
		eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

		LOGI("Native id %d", format);
		ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);

		surface = eglCreateWindowSurface(display, config, nativeWindow, NULL);

		const EGLint attribs2[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2, 
			EGL_NONE, EGL_NONE
		};

		LOGI("Surface %p", surface);

		context = eglCreateContext(display, config, NULL, attribs2);

		LOGI("Context %p", context);
		if(!context) {
			LOGI("NO CONTEXT!");
			exit(0);
		}


		if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
			LOGI("Unable to eglMakeCurrent");
			return -1;
		}

		eglQuerySurface(display, surface, EGL_WIDTH, &w);
		eglQuerySurface(display, surface, EGL_HEIGHT, &h);

		screenWidth = w;
		screenHeight = h;
		eglDisplay = display;
		eglContext = context;
		eglSurface = surface;
		this->nativeWindow = nativeWindow;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		LOGI("Done");
		return 0;
	}

	bool pollEvents() {

		int ident;
		int events;
		struct android_poll_source* source;


		if((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
			// Process this event.
			if (source != NULL) {
				source->process(app, source);
			}
			// Check if we are exiting.
			if (app->destroyRequested != 0) {
				closeWindow();
				return false;
			}
		}
/*
		if(tEvent && !touchEvents.empty()) {
			*tEvent = touchEvents.front();
			touchEvents.pop();
			return true;
		}*/
			return true;
	}


	bool flip() {
		//getEvent(nullptr);
		if(focus && eglDisplay != EGL_NO_DISPLAY) {
			eglSwapBuffers(eglDisplay, eglSurface);
			eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &screenWidth);
			eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &screenHeight);
		}

		return (eglDisplay == EGL_NO_DISPLAY);
	}

	void closeWindow() {
		if(eglDisplay != EGL_NO_DISPLAY) {
			eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			if(eglContext != EGL_NO_CONTEXT) {
				eglDestroyContext(eglDisplay, eglContext);
			}
			if(eglSurface != EGL_NO_SURFACE) {
				eglDestroySurface(eglDisplay, eglSurface);
			}
			eglTerminate(eglDisplay);
		}
		eglDisplay = EGL_NO_DISPLAY;
		eglContext = EGL_NO_CONTEXT;
		eglContext = EGL_NO_SURFACE;
	}

	void setFocus(bool focus) {
		this->focus = focus;
	}

	bool getFocus() const { return focus; }

	bool ready() { pollEvents(); return nativeWindow != nullptr; }
	int width() { return screenWidth; }
	int height() { return screenHeight; }

	struct android_app* app;

	std::deque<TouchEvent> touchEvents;
	std::deque<KeyEvent> keyEvents;


protected:

	bool focus;

	std::string title;
	//std::queue<TouchEvent> touchEvents;


	int screenWidth;
	int screenHeight;

	ANativeWindow *nativeWindow;

	EGLConfig eglConfig;

	EGLContext eglContext;
	EGLDisplay eglDisplay;
	EGLSurface eglSurface;

	//std::vector<unsigned int> pos;

	int program;

	float ppmm;

};

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	AndroidHost* host = static_cast<AndroidHost*>(app->userData);
	return host->handleAndroidEvent(event);
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	//struct engine* engine = (struct engine*)app->userData;

	AndroidHost *host = (AndroidHost*)app->userData;

	LOGI("Got cmd %d", cmd);

	switch (cmd) {
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.  Do so.
			//engine->app->savedState = malloc(sizeof(struct saved_state));
			//*((struct saved_state*)engine->app->savedState) = engine->state;
			//engine->app->savedStateSize = sizeof(struct saved_state);
			break;
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			LOGI("Init window");
			if(app->window != NULL) {
				host->initScreen(app->window);
			}
			break;
		case APP_CMD_TERM_WINDOW:
			LOGI("Terminating");
			host->closeWindow();
			exit(0);
			break;
		case APP_CMD_GAINED_FOCUS:
			// Start rendering
			LOGI("Got focus");
			host->setFocus(true);
			break;
		case APP_CMD_LOST_FOCUS:
			// Stop rendering
			LOGI("Lost focus");
			host->setFocus(false);
			break;
		case APP_CMD_CONFIG_CHANGED:
			// Resize here
			break;
	}
}

static AndroidHost host;


namespace grappix {

Window::click Window::NO_CLICK = { -1, -1, -1};

static function<void()> renderLoopFunction;
static function<void(uint32_t)> renderLoopFunction2;
static bool keyboardOn = false;

void debug_callback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, void* userParam) {
	LOGD("GLDEBUG:%s", message);
}

Window::Window() : RenderTarget(), winOpen(false), bmCounter(0) {
	frameBuffer = 0;
}

void Window::open(bool fs) {
	startTime = chrono::high_resolution_clock::now();
	_width = host.width();
	_height = host.height();
	winOpen = true;
	return;
}

void Window::open(int w, int h, bool fs) {
	startTime = chrono::high_resolution_clock::now();
	_width = host.width();
	_height = host.height();
	winOpen = true;
	return;
};

void Window::vsync() {
}

void Window::flip() {
	host.flip();
	auto t = chrono::high_resolution_clock::now();
	auto ms = chrono::duration_cast<chrono::microseconds>(t - startTime).count();
	tween::Tween::updateTweens(ms / 1000000.0f);

	//glfwSwapBuffers();
	//if(glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED)) {
	//	glfwCloseWindow();
	//	winOpen = false;			
	//}
}

void Window::render_loop(function<void(uint32_t)> f, int fps) {
	renderLoopFunction2 = f;
}

void Window::benchmark() {
	benchStart = chrono::high_resolution_clock::now();
	bmCounter = 100;
}

/*
unordered_map<int, int> Window::translate = {
	{ ENTER, GLFW_KEY_ENTER },
	{ LEFT, GLFW_KEY_LEFT },
	{ RIGHT, GLFW_KEY_RIGHT },
	{ UP, GLFW_KEY_UP },
	{ DOWN, GLFW_KEY_DOWN }
};*/

void init_keyboard(ANativeActivity *activity) {
	if(!keyboardOn)
		ANativeActivity_showSoftInput(activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT);
	keyboardOn = true;
}

unordered_map<int, int> Window::translate = {
	//{ F1, GLFW_KEY_F1 },
	{ BACKSPACE, AKEYCODE_DEL },
	{ ENTER, AKEYCODE_ENTER },
	//{ ESCAPE, AKEYCODE_ESCAPE },
	{ TAB, AKEYCODE_TAB },
	{ SPACE, AKEYCODE_SPACE },
	{ LEFT, AKEYCODE_DPAD_LEFT },
	{ RIGHT, AKEYCODE_DPAD_RIGHT },
	{ UP, AKEYCODE_DPAD_UP },
	{ DOWN, AKEYCODE_DPAD_DOWN },
	{ PAGEUP, AKEYCODE_PAGE_UP },
	{ PAGEDOWN, AKEYCODE_PAGE_DOWN },
	{ ALT_LEFT, AKEYCODE_ALT_LEFT },
	{ ALT_RIGHT, AKEYCODE_ALT_RIGHT },
	{ SHIFT_LEFT, AKEYCODE_SHIFT_LEFT },
	{ SHIFT_RIGHT, AKEYCODE_SHIFT_RIGHT },
	{ ESCAPE, 111 },
	{ DELETE, 112 },
	{ CTRL, 113 },
	{ WINDOWS, 117},
	{ HOME, 122},
	{ END, 123},
};
bool Window::key_pressed(key k) {
	init_keyboard(host.app->activity);
	//auto glfwKey = translate[k];
	return false;
	//return glfwGetKey(glfwKey) != 0;
}

Window::click Window::get_click() {
	/*if(click_buffer.size() > 0) {
		auto k = click_buffer.front();
		click_buffer.pop_front();
		return k;
	} */
	if(host.touchEvents.size() > 0) {
		auto t = host.touchEvents.front();
		host.touchEvents.pop_front();
		if(t.what == TouchEvent::DOWN)
			return click(t.x, t.y, 1);
	}

	return NO_CLICK;
}



Window::key Window::get_key() {
	init_keyboard(host.app->activity);
	if(host.keyEvents.size() > 0) {
		auto e = host.keyEvents.front();
		int k = e.code;
		host.keyEvents.pop_front();
		//LOGD("Code %d", k);
		if(e.what == KeyEvent::DOWN) {
			if(k >= 29 && k <= 54)
				return (key)(k - 29 + 'a');
			for(auto t : translate) {
				if(t.second == k) {
					//LOGD("Translated to %d", t.first);
					return (key)t.first;
				}
			}
			return (key)k;
		}
	}	
	return NO_KEY;
}

Window screen;
}

void extract_files(struct android_app* app) {

	AAssetManager* mgr = app->activity->assetManager;

	const char *path = app->activity->internalDataPath;
	LOGD("Path %s (%s)", path, app->activity->externalDataPath);
	chdir(path);
	mkdir("data", 0777);
	AAssetDir *assetDir = AAssetManager_openDir(mgr, "data");
	const char *filename;
	while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
		LOGD("Found %s", filename);
	    string s = string("data/") + filename;
	    AAsset *asset = AAssetManager_open(mgr, s.c_str(), AASSET_MODE_STREAMING);
	    char buf[16*1024];
	    int nb_read = 0;
	    //utils::makedirs(filename);
	    FILE *out = fopen(s.c_str(), "wb");
	    while((nb_read = AAsset_read(asset, buf, sizeof(buf))) > 0)
	        fwrite(buf, nb_read, 1, out);
	    fclose(out);
	    AAsset_close(asset);
	}
	AAssetDir_close(assetDir);
}

void android_main(struct android_app* app) {
	// Make sure glue isn't stripped.
	app_dummy();
	extract_files(app);
	LOGD("App started");
	host.app = app;
	app->userData = &host;
	app->onAppCmd = engine_handle_cmd;
	app->onInputEvent = engine_handle_input;
	while(!host.ready()) {
		usleep(500);
	}
	try {
		main(0, nullptr);
	} catch (std::exception &e) {
		LOGD("Caught exception: %s", e.what());
	}
	LOGD("At exit");
	auto lastMs = utils::getms();
	while(grappix::screen.is_open()) {
		auto ms = utils::getms();
		uint32_t rate = ms - lastMs;
		lastMs = ms;
		host.pollEvents();
		grappix::renderLoopFunction2(rate);
	}
	LOGD("App ending!");

}
