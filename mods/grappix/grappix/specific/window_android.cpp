#include "../window.h"
//#include "../resources.h"
//#include <tween/tween.h>
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

bool initEGL(EGLConfig& eglConfig, EGLContext& eglContext, EGLDisplay& eglDisplay, EGLSurface &eglSurface, EGLNativeWindowType nativeWin);


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
	AndroidHost() : app(nullptr), focus(true), nativeWindow(nullptr) {}

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
					/*int x = AMotionEvent_getX(event, i);
					int y = AMotionEvent_getY(event, i);
					int id = AMotionEvent_getPointerId(event, i);

					int xy = x | y<<16;
*/
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

	void display_keyboard(bool pShow) {
	    // Attaches the current thread to the JVM.
	    jint lResult;
	    jint lFlags = 0;

	    JavaVM* lJavaVM = app->activity->vm;
	    JNIEnv* lJNIEnv = app->activity->env;

	    JavaVMAttachArgs lJavaVMAttachArgs;
	    lJavaVMAttachArgs.version = JNI_VERSION_1_6;
	    lJavaVMAttachArgs.name = "NativeThread";
	    lJavaVMAttachArgs.group = NULL;

	    lResult=lJavaVM->AttachCurrentThread(&lJNIEnv, &lJavaVMAttachArgs);
	    if (lResult == JNI_ERR) {
	        return;
	    }

	    // Retrieves NativeActivity.
	    jobject lNativeActivity = app->activity->clazz;
	    jclass ClassNativeActivity = lJNIEnv->GetObjectClass(lNativeActivity);

	    // Retrieves Context.INPUT_METHOD_SERVICE.
	    jclass ClassContext = lJNIEnv->FindClass("android/content/Context");
	    jfieldID FieldINPUT_METHOD_SERVICE =
	        lJNIEnv->GetStaticFieldID(ClassContext,
	            "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
	    jobject INPUT_METHOD_SERVICE =
	        lJNIEnv->GetStaticObjectField(ClassContext,
	            FieldINPUT_METHOD_SERVICE);
	    //jniCheck(INPUT_METHOD_SERVICE);

	    // Runs getSystemService(Context.INPUT_METHOD_SERVICE).
	    jclass ClassInputMethodManager = lJNIEnv->FindClass(
	        "android/view/inputmethod/InputMethodManager");
	    jmethodID MethodGetSystemService = lJNIEnv->GetMethodID(
	        ClassNativeActivity, "getSystemService",
	        "(Ljava/lang/String;)Ljava/lang/Object;");
	    jobject lInputMethodManager = lJNIEnv->CallObjectMethod(
	        lNativeActivity, MethodGetSystemService,
	        INPUT_METHOD_SERVICE);

	    // Runs getWindow().getDecorView().
	    jmethodID MethodGetWindow = lJNIEnv->GetMethodID(
	        ClassNativeActivity, "getWindow",
	        "()Landroid/view/Window;");
	    jobject lWindow = lJNIEnv->CallObjectMethod(lNativeActivity,
	        MethodGetWindow);
	    jclass ClassWindow = lJNIEnv->FindClass(
	        "android/view/Window");
	    jmethodID MethodGetDecorView = lJNIEnv->GetMethodID(
	        ClassWindow, "getDecorView", "()Landroid/view/View;");
	    jobject lDecorView = lJNIEnv->CallObjectMethod(lWindow,
	        MethodGetDecorView);

	    if (pShow) {
	        // Runs lInputMethodManager.showSoftInput(...).
	        jmethodID MethodShowSoftInput = lJNIEnv->GetMethodID(
	            ClassInputMethodManager, "showSoftInput",
	            "(Landroid/view/View;I)Z");
	        jboolean lResult = lJNIEnv->CallBooleanMethod(
	            lInputMethodManager, MethodShowSoftInput,
	            lDecorView, lFlags);
	    } else {
	        // Runs lWindow.getViewToken()
	        jclass ClassView = lJNIEnv->FindClass(
	            "android/view/View");
	        jmethodID MethodGetWindowToken = lJNIEnv->GetMethodID(
	            ClassView, "getWindowToken", "()Landroid/os/IBinder;");
	        jobject lBinder = lJNIEnv->CallObjectMethod(lDecorView,
	            MethodGetWindowToken);

	        // lInputMethodManager.hideSoftInput(...).
	        jmethodID MethodHideSoftInput = lJNIEnv->GetMethodID(
	            ClassInputMethodManager, "hideSoftInputFromWindow",
	            "(Landroid/os/IBinder;I)Z");
	        jboolean lRes = lJNIEnv->CallBooleanMethod(
	            lInputMethodManager, MethodHideSoftInput,
	            lBinder, lFlags);
	    }

	    // Finished with the JVM.
	    lJavaVM->DetachCurrentThread();
	}

	int initScreen(ANativeWindow *nativeWindow) {

		if(eglContext == nullptr) {
			initEGL(eglConfig, eglContext, eglDisplay, eglSurface, nativeWindow);
		} else {
			eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, nullptr);
			if(eglSurface == EGL_NO_SURFACE) {
				LOGI("NO SURFACE!");
			}

			if(eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) == EGL_FALSE) {
				LOGI("NO MAKE CURRENT!");
			}
		}
/*
		eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, NULL);


		LOGI("Surface %p", eglSurface);

		if (eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) == EGL_FALSE) {
			LOGI("Unable to eglMakeCurrent");
			return -1;
		}
*/
		EGLint w, h;

		eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &w);
		eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &h);

		screenWidth = w;
		screenHeight = h;
		this->nativeWindow = nativeWindow;

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
		LOGD("Closing window");
		if(eglDisplay != EGL_NO_DISPLAY) {
			//eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			/*if(eglContext != EGL_NO_CONTEXT) {
				eglDestroyContext(eglDisplay, eglContext);
			}
			if(eglSurface != EGL_NO_SURFACE) {
				eglDestroySurface(eglDisplay, eglSurface);
			}
			eglTerminate(eglDisplay); */
		}
		//eglDisplay = EGL_NO_DISPLAY;
		//eglContext = EGL_NO_CONTEXT;
		//eglSurface = EGL_NO_SURFACE;
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

	switch (cmd) {
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.  Do so.
			//engine->app->savedState = malloc(sizeof(struct saved_state));
			//*((struct saved_state*)engine->app->savedState) = engine->state;
			//engine->app->savedStateSize = sizeof(struct saved_state);
			LOGI("Save state");
			break;
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			LOGI("Init window");
			if(app->window != NULL) {
				host->initScreen(app->window);
			}
			break;
		case APP_CMD_PAUSE:
			LOGI("Pause");
			break;
		case APP_CMD_TERM_WINDOW:
			LOGI("Terminating");
			host->closeWindow();
			app->window = nullptr;
			//exit(0);
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
			LOGI("Resize");
			// Resize here
			break;
		case APP_CMD_DESTROY:
			LOGI("Destroy!");
			exit(0);
			break;
		default:
			LOGI("Got cmd %d", cmd);
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
	setup(host.width(), host.height());
}

void Window::open(int w, int h, bool fs) {
	setup(host.width(), host.height());
};

void Window::vsync() {
}

void Window::flip() {
	host.flip();
	//auto t = chrono::high_resolution_clock::now();
	//auto ms = chrono::duration_cast<chrono::microseconds>(t - startTime).count();
	//Resources::getInstance().update();
	//tween::Tween::updateTweens(ms / 1000000.0f);
}

void Window::render_loop(function<void(uint32_t)> f, int fps) {
	renderLoopFunction2 = f;
}

void init_keyboard(ANativeActivity *activity) {
	if(!keyboardOn)
		host.display_keyboard(true);
		//ANativeActivity_showSoftInput(activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT);
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
	{ CTRL_LEFT, 113 },
	{ WINDOWS, 117},
	{ HOME, 122},
	{ END, 123},
};
bool Window::key_pressed(key k) {
	//init_keyboard(host.app->activity);
	//auto glfwKey = translate[k];
	return false;
	//return glfwGetKey(glfwKey) != 0;
}

Window::click Window::get_click(bool peek) {
	/*if(click_buffer.size() > 0) {
		auto k = click_buffer.front();
		click_buffer.pop_front();
		return k;
	} */
	if(host.touchEvents.size() > 0) {
		auto t = host.touchEvents.front();
		if(!peek)
			host.touchEvents.pop_front();
		if(t.what == TouchEvent::DOWN)
			return click(t.x, t.y, 1);
	}

	return NO_CLICK;
}

bool Window::mouse_pressed() {
	return false;//glfwGetMouseButton(0);
}

tuple<int, int> Window::mouse_position() {
	int x,y;
	x = y = 0;
	return make_tuple(x, y);
}



Window::key Window::get_key(bool peek) {
	//init_keyboard(host.app->activity);
	if(host.keyEvents.size() > 0) {
		auto e = host.keyEvents.front();
		int k = e.code;
		if(!peek)
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

std::shared_ptr<Window> screenptr = make_shared<Window>();
Window& screen = *screenptr;
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

//static std::vector<std::pair<std::function<void(void*)>, void*>> callbacks;
//void add_callback(std::function<void(void*)> f, void *context) {
//	callbacks.push_back(std::make_pair(f, context));
//}

void android_main(struct android_app* app) {

	using grappix::screen;
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
		char *args[1] = { "grappix" };
		main(1, args);
	} catch (std::exception &e) {
		LOGD("Caught exception: %s", e.what());
	}
	LOGD("At exit");
	auto lastMs = utils::getms();
	bool lastFocus = host.getFocus();
	while(grappix::screen.is_open()) {
		screen.update_callbacks();
		auto ms = utils::getms();
		uint32_t rate = ms - lastMs;
		lastMs = ms;
		host.pollEvents();
		if(host.getFocus() != lastFocus) {
			LOGD("Focus changed");
			lastFocus = host.getFocus();
			if(lastFocus && screen.focus_func)
				screen.focus_func();
			if(!lastFocus && screen.focus_lost_func)
				screen.focus_lost_func();
		}

		//for(auto f : callbacks) {
		//	f.first(f.second);
		//}
		//if(host.getFocus())
		grappix::renderLoopFunction2(rate);
	}
	LOGD("App ending!");

}
