#include <stdio.h>
#include <stdlib.h>

/* Include Screen */
#include <screen/screen.h>

int main(void) {
	int err = 0;
	screen_context_t   screen_context = 0;
	screen_window_t    screen_window = 0;

	/* Create screen context
	 *
	 * The first step for all screen applications is to create a "screen context".
	 * This context establishes a connection between the application and the screen subsystem.
	 * Without this connection the application cannot communicate with screen to access any functionality.
	 *
	 * -screen_context is the handle to the created context.
	 * -SCREEN_APPLICATION_CONTEXT - is the type of context that we want to open, this context allows us
	 * to create our own windows and control some of its properties. The application won't be able to modify windows
	 * that were created by other applications. For a complete list on all the context types go to:
	 * http://www.qnx.com/developers/docs/7.0.0/index.html#com.qnx.doc.screen/topic/group__screen__contexts_1Screen_Context_Types.html
	 *
	 * */
	err = screen_create_context(&screen_context, SCREEN_APPLICATION_CONTEXT);
	if(err != 0){
		printf("Failed to create screen context\n");
	}

	/* Get display information
	 *
	 * We need to get the display dimensions.
	 * */
	int disp_count = 0;
	screen_get_context_property_iv(screen_context, SCREEN_PROPERTY_DISPLAY_COUNT, &disp_count);
	screen_display_t *screen_disps = calloc(disp_count, sizeof(screen_display_t));
	screen_get_context_property_pv(screen_context, SCREEN_PROPERTY_DISPLAYS, (void **)screen_disps);
	screen_display_t screen_disp = screen_disps[0];
	free(screen_disps);
	int disp_dims[2] = {0, 0};
	screen_get_display_property_iv(screen_disp, SCREEN_PROPERTY_SIZE, disp_dims);

	/* Create window
	 *
	 * We need to create a window as our render target to present graphics in the display.
	 * Windows are the only render targets that allow to output our graphics in a physical monitor.
	 * */
	err = screen_create_window(&screen_window, screen_context);
	if(err != 0){
			printf("Failed to create screen window\n");
	}

	/* Set window properties
	 *
	 * Set the R/W usage properties since the buffer will be accessed by the CPU.
	 * */
	int usage = SCREEN_USAGE_READ | SCREEN_USAGE_WRITE;
	err = screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_USAGE, &usage);
	if(err != 0){
			printf("Failed to set usage property\n");
	}

	/* Set window buffer properties */
	int buffer[2] = {1, 1};
	err = screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_BUFFER_SIZE, buffer);
	if(err != 0){
		printf("Failed to set buffer size\n");
	}
	/* Set background color ABGR*/
	int color = 0x000000FF;
	err = screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_COLOR, &color);
	if(err != 0){
		printf("Failed to set color\n");
	}

	/* Set Source size to match the display dimensions */
	screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_SOURCE_SIZE, disp_dims);

	/* Set the position of the buffer outside of the display
	 * All things outside of the buffer will be filled out with the
	 * color selected above.
	 * */
	int pos[2] = { -disp_dims[0], -disp_dims[1] };
	screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_SOURCE_POSITION, pos);

	/* Create window buffer
	 * This is the buffer object where our graphics will go to.
	 * */
	err = screen_create_window_buffers(screen_window, 1);
	if(err != 0){
			printf("Failed to create window buffer\n");
	}

	/* Get buffer dimensions */
	err = screen_get_window_property_iv(screen_window, SCREEN_PROPERTY_BUFFER_SIZE, buffer);
	if(err != 0){
			printf("Failed to get window buffer size\n");
	}

	/* Get the buffer handle */
	screen_buffer_t screen_buf;
	err = screen_get_window_property_pv(screen_window, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&screen_buf);
	if(err != 0){
			printf("Failed to get window buffer\n");
	}

	/* Get the buffer pointer
	 *
	 * First we got the buffer object handle, in order to render we need to get
	 * the pointer to the actual buffer where the data to be displayed is stored.
	 * */
	int *ptr = NULL;
	err = screen_get_buffer_property_pv(screen_buf, SCREEN_PROPERTY_POINTER, (void **)&ptr);
	if(err != 0){
			printf("Failed to get buffer pointer\n");
	}

	/* Post buffer to be displayed
	 *
	 * Post the buffer to be displayed.
	 * screen_window - Handle of the window object
	 * screen_buf - Handle of the buffer object to be displayed
	 * count - The number of rectangles provided in the dirty_rects argument,
	 *         or 0 to indicate that the entire buffer is dirty.
	 * dirty_rects - An array of integers containing the x and y coordinates, width, and height of a rectangle
	 *               that bounds the area of the rendering buffer that has changed since the last posting of the window.
	 *               The dirty_rects argument must provide at least count * 4 integers.
	 *               You can also pass NULL if you are passing 0 for the count argument.
	 * flags- 0 to return immediately if there are render buffers available.
	 *
	 * */
	err = screen_post_window(screen_window, screen_buf, 0, NULL, 0);
	if(err != 0){
			printf("Failed to post window\n");
	}

	/* Trap execution */
	while(1){}

	screen_destroy_window(screen_window);
	screen_destroy_context(screen_context);

	return EXIT_SUCCESS;
}
