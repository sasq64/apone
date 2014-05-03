
#include <EGL/egl.h>
#include <coreutils/log.h>


bool initEGL(EGLConfig& eglConfig, EGLContext& eglContext, EGLDisplay& eglDisplay) {

	EGLint format;
	EGLint numConfigs;
	EGLConfig config;
	EGLConfig configList[32];
	EGLContext context;

	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	EGLint m0;
	EGLint m1;

	eglInitialize(eglDisplay, &m0, &m1);

	LOGI("EGL v%d.%d", m0, m1);	

	/* Here, the application chooses the configuration it desires. In this
	 * sample, we have a very simplified selection process, where we pick
	 * the first EGLConfig that matches our criteria */
	eglGetConfigs(eglDisplay, configList, 32, &numConfigs);

	LOGI("Found %d matching configs", numConfigs);

	for(int i=0; i<numConfigs; i++) {
		EGLint conf, id, stype, redSize, caveat, sbuffers;
		eglGetConfigAttrib(eglDisplay, configList[i], EGL_CONFORMANT, &conf);
		eglGetConfigAttrib(eglDisplay, configList[i], EGL_CONFIG_ID, &id);
		eglGetConfigAttrib(eglDisplay, configList[i], EGL_SURFACE_TYPE, &stype);
		eglGetConfigAttrib(eglDisplay, configList[i], EGL_RED_SIZE, &redSize);
		eglGetConfigAttrib(eglDisplay, configList[i], EGL_CONFIG_CAVEAT, &caveat);
		eglGetConfigAttrib(eglDisplay, configList[i], EGL_SAMPLE_BUFFERS, &sbuffers);
						
		LOGI("Config %d (%d) conformant %x RED %d caveat %x stype %x", i, id, conf, redSize, caveat, stype);

		if((conf & EGL_OPENGL_ES2_BIT) && (stype & EGL_WINDOW_BIT)) {
			config = configList[i];
			if(sbuffers > 0) {
				break;
			}
		}
	}

	const EGLint attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2, 
		EGL_NONE, EGL_NONE
	};

	eglContext = eglCreateContext(eglDisplay, config, NULL, attribs);

	LOGI("Context %p", eglContext);
	if(!eglContext) {
		LOGI("NO CONTEXT!");
		return false;
	}
	return true;
}
