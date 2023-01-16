#include <stdio.h>
#include <stdlib.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <sys/keycodes.h>
#include <screen/screen.h>
#include <input/screen_helpers.h>

int screen_width = 500;
int screen_height = 500;
EGLint interval = 1;

GLshort points[4];

int main(void)
{
	screen_context_t screen_ctx;
	screen_window_t screen_win;
	screen_create_context(&screen_ctx, SCREEN_APPLICATION_CONTEXT);
	screen_create_window(&screen_win, screen_ctx);

	// set appropriate properties
	/* Indicate that OpenGL ES 2.x will render to the buffer associated with this render target. */
	int usage = SCREEN_USAGE_OPENGL_ES2;
	int format = SCREEN_FORMAT_RGBA8888;
	int interval = 1;
	int size[2] = {screen_width, screen_height}; /* Size of window */
	int pos[2] = {0, 0};	  /* Position of window */
	screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
	screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &format); /* must match egl configuration*/
	screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SWAP_INTERVAL, &interval);
	screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SIZE, size);
	screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_POSITION, pos);

	// create buffer for render target
	int nbuffers = 2; /* Number of window buffers */
	screen_create_window_buffers(screen_win, nbuffers);

	// create render context
	EGLDisplay egl_disp;

	egl_disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (egl_disp == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS) {
 		printf("get egl display failed\n");
	return 1;
	}
	eglInitialize(egl_disp, NULL, NULL);

	// choose egl configuration
	// use eglGetConig
	EGLint attribList[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	    EGL_RED_SIZE, 8,
	    EGL_GREEN_SIZE, 8,
	    EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
	    EGL_NONE
	};
	EGLConfig egl_configs[10];
	EGLint egl_num_configs;
	eglChooseConfig(egl_disp, attribList, egl_configs, 10, &egl_num_configs);

	EGLConfig egl_conf = egl_configs[0]; /* Resulting EGL config */

	// create egl surface
	EGLSurface egl_surf;
	const EGLint egl_surf_attr[] = {
		EGL_RENDER_BUFFER,
		EGL_BACK_BUFFER,
		EGL_NONE
	};
	egl_surf = eglCreateWindowSurface(egl_disp, egl_conf, screen_win, egl_surf_attr);
	if (egl_surf == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS) {
		printf("failed to create suface\n");
		return 1;
	}

	// create rendering context
	const EGLint egl_context_attrs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLContext egl_ctx;
	egl_ctx = eglCreateContext(egl_disp, egl_conf, EGL_NO_CONTEXT, egl_context_attrs);
	if (egl_ctx == EGL_NO_CONTEXT || eglGetError() != EGL_SUCCESS) {
		printf("failed to create egl context\n");
		return 1;
	}

	// bind render target
	eglMakeCurrent(egl_disp, egl_surf, egl_surf, egl_ctx);
	int rc = eglSwapInterval(egl_disp, interval);
	if (rc != EGL_TRUE) {
		return -1;
	}

	glClearColor(1.0f, 0.0f, 0.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);


	eglSwapBuffers(egl_disp, egl_surf);

	while(1) {
	}

	return EXIT_SUCCESS;
}
