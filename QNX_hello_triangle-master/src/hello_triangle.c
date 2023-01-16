#include <stdio.h>
#include <stdlib.h>

/* Include Screen */
#include <screen/screen.h>

/* Include EGL/OpenGL ES */
#include <EGL/egl.h>
#include <GLES3/gl3.h>

struct egl_conf_attr{
	EGLint surface_type;
	EGLint red_size;
	EGLint green_size;
	EGLint blue_size;
	EGLint alpha_size;
	EGLint samples;
	EGLConfig config_id;
};

const char *vertex_shader_source = "#version 300 es\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragment_shader_source = "#version 300 es\n"
    "out lowp vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

struct egl_conf_attr egl_conf_attr_sel = {
		.surface_type = EGL_WINDOW_BIT,   /* Ask for displayable and pbuffer surfaces */
		.red_size = 8,        /* Minimum number of red bits per pixel */
		.green_size = 8,      /* Minimum number of green bits per pixel */
		.blue_size = 8,       /* Minimum number of blue bits per pixel */
		.alpha_size = 8,      /* Minimum number of alpha bits per pixel */
		.samples = EGL_DONT_CARE,         /* Minimum number of samples per pixel */
		.config_id = 0,       /* used to get a specific EGL config */
};

/* Query for the best EGL configuration
 *
 * This function queries for the best EGL configuration that matches
 * egl_conf.
 * */
EGLBoolean select_EGL_config(EGLDisplay egl_display, struct egl_conf_attr *egl_conf);

void checkCompileStatus(GLuint shader){
	
	GLint val;
	GLboolean comp_sup;
	/* Check if shader compiler is supported */
	glGetBooleanv(GL_SHADER_COMPILER, &comp_sup);
	if(comp_sup == GL_TRUE){
		/* Check compile status */
		glGetShaderiv(shader, GL_COMPILE_STATUS, &val);
		if(val == GL_TRUE){
			printf("Shader compile was successful\n");
		} else{
			char infolog[512];
			printf("Shader compile failed\n");
			glGetShaderInfoLog(shader, 512, NULL, infolog);
			printf(infolog);
		}
	} else {
		printf("Shader compilation is not supported\n");
	}
}


int main(void) {
	int err = 0;
	screen_context_t   screen_context = 0;
	screen_window_t    screen_window = 0;

	/*************************************** Configure Screen subsystem *****************************************/
	/* Create screen context */
	err = screen_create_context(&screen_context, SCREEN_APPLICATION_CONTEXT);
	if(err != 0){
		printf("Failed to create screen context\n");
	}

	/* Get display information  */
	int disp_count = 0;
	screen_get_context_property_iv(screen_context, SCREEN_PROPERTY_DISPLAY_COUNT, &disp_count);
	screen_display_t *screen_disps = calloc(disp_count, sizeof(screen_display_t));
	screen_get_context_property_pv(screen_context, SCREEN_PROPERTY_DISPLAYS, (void **)screen_disps);
	screen_display_t screen_disp = screen_disps[0];
	free(screen_disps);
	int disp_dims[2] = {0, 0};
	screen_get_display_property_iv(screen_disp, SCREEN_PROPERTY_SIZE, disp_dims);

	/* Create window */
	err = screen_create_window(&screen_window, screen_context);
	if(err != 0){
			printf("Failed to create screen window\n");
	}

	/* Set window properties to work with OpenGL ES 3.0 */
	int usage =  SCREEN_USAGE_OPENGL_ES2 | SCREEN_USAGE_OPENGL_ES3;
	err = screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_USAGE, &usage);
	if(err != 0){
			printf("Failed to set usage property\n");
	}

	/* Create window buffers */
	err = screen_create_window_buffers(screen_window, 2);
	if(err != 0){
			printf("Failed to create window buffer\n");
	}

	/* Get buffer dimensions */
	int buffer[2];
	err = screen_get_window_property_iv(screen_window, SCREEN_PROPERTY_BUFFER_SIZE, buffer);
	if(err != 0){
			printf("Failed to get window buffer size\n");
	}

	/******************************************** Configure EGL ***********************************************/
	EGLDisplay egl_disp;
	/* Get EGL display */
	egl_disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(egl_disp == EGL_NO_DISPLAY){
		printf("No display connection matched EGL_DEFAULT_DISPLAY\n");
	}
	EGLint major, minor;
	EGLBoolean ret;
	/* Initialize EGL display */
	ret = eglInitialize(egl_disp, &major, &minor);
	if(ret == EGL_FALSE){
		printf("EGL initialization failed!\n");
	} else{
		printf("EGL %d.%d initialization succeeded!\n", major, minor);
	}

	/* Select EGL configuration
	 * If select_EGL_config returns true the configuration will be stored
	 * at egl_conf_attr_sel.config_id.
	 * */
	ret = select_EGL_config(egl_disp, &egl_conf_attr_sel);
	if(ret == EGL_FALSE){
		printf("Failed to select EGL configuration\n");
	}

	/* Create EGL rendering context */
    struct {
            EGLint client_version[2];
            EGLint none;
    } egl_ctx_attr = {
            .client_version = { EGL_CONTEXT_CLIENT_VERSION, 3 },
            .none = EGL_NONE
    };
	EGLContext egl_context;
	egl_context = eglCreateContext(egl_disp, egl_conf_attr_sel.config_id, EGL_NO_CONTEXT, (EGLint*)&egl_ctx_attr);
	if(egl_context == EGL_NO_CONTEXT){
		printf("EGL context creation failed\n");
	}

	/* Create EGL window surface */
	EGLSurface egl_surf;
	egl_surf = eglCreateWindowSurface(egl_disp, egl_conf_attr_sel.config_id, screen_window, NULL);
	if(egl_surf == EGL_NO_SURFACE){
		printf("Failed to create EGL window surface, Error: %d\n", eglGetError());
	}

	/* Attach EGL rendering context to the surface
	 * All OpenGL ES calls will be executed on the context/surface that is currently bind.
	 * egl_surf is used for both reading and writing.
	 * */
	ret = eglMakeCurrent(egl_disp, egl_surf, egl_surf, egl_context);
	if(ret == EGL_FALSE){
		printf("Failed to bind EGL context/surface\n");
	}

	/* Set swap interval
	 * This function specifies the minimum number of video frame periods per buffer swap
	 * for the window associated with the current context. So, if the interval is 0,
	 * the application renders as fast as it can.
	 * Interval values of 1 or more limit the rendering to fractions of the display's refresh rate.
	 * (For example, 60, 30, 20, 15, etc. frames per second in the case of a display with a refresh rate of 60 Hz.)
	 * */
	ret = eglSwapInterval(egl_disp, 1);
	if(ret == EGL_FALSE){
		printf("Failed to set Swap interval\n");
	}

	/**************************************** Setup OpenGL ES ***************************************/
	/* Create Vertex shader */
	GLuint vertex_shader;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	checkCompileStatus(vertex_shader);

	/* Create Fragment shader */
	GLuint fragment_shader;
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	checkCompileStatus(fragment_shader);

	/* Create program */
	GLuint program;
	program = glCreateProgram();
	if(program == 0){
		printf("Failed to create program\n");
	}
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	/* Check link status*/
	GLint link_status;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if(link_status == GL_FALSE){
		printf("Failed to link program\n");
	}

	/* Delete shader programs */
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	/* Create Vertex Array Object*/
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	/* Create Vertex Buffer Object */
	GLuint VBO;
	glGenBuffers(1, &VBO);

	/* Bind newly created Vertex Buffer Object to VAO */
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	/* Copy Vertices to buffer object */
	/* Triangle vertices */
	float triangle_vert[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vert), triangle_vert, GL_STATIC_DRAW);

	/* Define array of vertex attribute data so that OpenGL ES knows how to interpret the data */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/**************************** Render Triangle ****************************/
	/* Use recently compiled program */
	glUseProgram(program);

    /* Set background color RGBA */
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

	/* Draw triangle */
	glDrawArrays(GL_TRIANGLES, 0, 3);

	/* Post changes */
	ret = eglSwapBuffers(egl_disp, egl_surf);
	if(ret == EGL_FALSE){
		printf("Failed to swap buffers\n");
	}

	/* Trap execution */
	while(1){}

	screen_destroy_window(screen_window);
	screen_destroy_context(screen_context);

	return EXIT_SUCCESS;
}


/* Query for the best EGL configuration
 *
 * This function queries for the best EGL configuration that matches
 * egl_conf.
 * */
EGLBoolean select_EGL_config(EGLDisplay egl_display, struct egl_conf_attr *egl_conf){

	EGLConfig *egl_configs;
	EGLint egl_num_configs;
	EGLBoolean ret;
	EGLBoolean config_found = EGL_FALSE;

	/* If a configuration ID is known already skip the loop and select the ID */
	if(egl_conf->config_id == 0){
		/* Get available EGL configurations */
		ret = eglGetConfigs(egl_display, NULL, 0, &egl_num_configs);
		if(ret == EGL_FALSE){
			printf("Failed to get EGL configurations\n");
		} else{
			printf("%d available configurations\n", egl_num_configs);
		}
		egl_configs = malloc(egl_num_configs * sizeof(EGLConfig));
		ret = eglGetConfigs(egl_display, egl_configs, egl_num_configs, &egl_num_configs);
		if(ret == EGL_FALSE){
			printf("Failed to get list of EGL configurations\n");
		}

		/* Loop through available configurations to select the best match */
		for(int i = 0; i < egl_num_configs; i++){
			EGLint egl_val;

			/* Get surface type */
			if(egl_conf->surface_type != EGL_DONT_CARE){
				ret = eglGetConfigAttrib(egl_display, egl_configs[i], EGL_SURFACE_TYPE, &egl_val);
				if(ret == EGL_FALSE){
					printf("Failed to get EGL surface type on config %d\n", i);
				} else if((egl_val & egl_conf->surface_type) != egl_conf->surface_type){
					continue;
				}
			}

			/* Get red_size */
			if(egl_conf->red_size != EGL_DONT_CARE){
				ret = eglGetConfigAttrib(egl_display, egl_configs[i], EGL_RED_SIZE, &egl_val);
				if(ret == EGL_FALSE){
					printf("Failed to get EGL red size on config %d\n", i);
				} else if(egl_val != egl_conf->red_size){
					continue;
				}
			}

			/* Get green size */
			if(egl_conf->green_size != EGL_DONT_CARE){
				ret = eglGetConfigAttrib(egl_display, egl_configs[i], EGL_GREEN_SIZE, &egl_val);
				if(ret == EGL_FALSE){
					printf("Failed to get EGL green size on config %d\n", i);
				} else if(egl_val != egl_conf->green_size){
					continue;
				}
			}

			/* Get blue size */
			if(egl_conf->blue_size != EGL_DONT_CARE){
				ret = eglGetConfigAttrib(egl_display, egl_configs[i], EGL_BLUE_SIZE, &egl_val);
				if(ret == EGL_FALSE){
					printf("Failed to get EGL blue size on config %d\n", i);
				} else if(egl_val != egl_conf->blue_size){
					continue;
				}
			}

			/* Get alpha size */
			if(egl_conf->alpha_size != EGL_DONT_CARE){
				ret = eglGetConfigAttrib(egl_display, egl_configs[i], EGL_ALPHA_SIZE, &egl_val);
				if(ret == EGL_FALSE){
					printf("Failed to get EGL alpha size on config %d\n", i);
				} else if(egl_val != egl_conf->alpha_size){
					continue;
				}
			}

			/* Get samples */
			if(egl_conf->samples != EGL_DONT_CARE){
				ret = eglGetConfigAttrib(egl_display, egl_configs[i], EGL_SAMPLES, &egl_val);
				if(ret == EGL_FALSE){
					printf("Failed to get EGL samples on config %d\n", i);
				} else if(egl_val != egl_conf->samples){
					continue;
				}
			}

			/* If we get to this point it means that we've matched all our criteria and we can
			 * stop searching
			 * */
			config_found = EGL_TRUE;
			egl_conf->config_id = egl_configs[i];
			free(egl_configs);
			break;
		}
	} else{
		config_found = EGL_TRUE;
	}

	return config_found;
}
