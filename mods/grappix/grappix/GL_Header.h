#if (defined ANDROID || defined RASPBERRYPI)
#include <GLES2/gl2.h>
#else
#define NOGDI
#include <GL/glew.h>
#endif
