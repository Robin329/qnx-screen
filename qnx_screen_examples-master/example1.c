#include <stdio.h>
#include <stdlib.h>

#include <screen/screen.h>

int main(void)
{
	int err = 0;
	screen_context_t   screen_context = 0;
	screen_window_t    screen_window = 0;

	err = screen_create_context(&screen_context, SCREEN_APPLICATION_CONTEXT);
	if(err != 0){
		printf("Failed to create screen context\n");
	}

	err = screen_create_window(&screen_window, screen_context);
	if(err != 0){
			printf("Failed to create screen window\n");
	}

	int usage = SCREEN_USAGE_READ | SCREEN_USAGE_WRITE;
	err = screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_USAGE, &usage);
	if(err != 0){
			printf("Failed to set usage property\n");
	}

	// set screen format to RGBA8888, 0xAARRGGBB
	int screenFormat = SCREEN_FORMAT_RGBA8888;
	screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_FORMAT, &screenFormat);
	if(err != 0){
			printf("Failed to set format property\n");
	}

#if 1
	// set screen z ordor to 10, 0 is bottom, n(>0) is top
	int zorder = 15;
	screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_ZORDER, &zorder);
	if(err != 0){
			printf("Failed to set zorder property\n");
	}
#endif

	err = screen_create_window_buffers(screen_window, 1);
	if(err != 0){
			printf("Failed to create window buffer\n");
	}

	int buffer[2];
	err = screen_get_window_property_iv(screen_window, SCREEN_PROPERTY_BUFFER_SIZE, buffer);
	if(err != 0){
			printf("Failed to get window buffer size\n");
	}

	screen_buffer_t screen_buf;
	err = screen_get_window_property_pv(screen_window, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&screen_buf);
	if(err != 0){
			printf("Failed to get window buffer\n");
	}

	int *ptr = NULL;
	err = screen_get_buffer_property_pv(screen_buf, SCREEN_PROPERTY_POINTER, (void **)&ptr);
	if(err != 0){
			printf("Failed to get buffer pointer\n");
	}

	int stride = 0;
	err = screen_get_buffer_property_iv(screen_buf, SCREEN_PROPERTY_STRIDE, &stride);
	if(err != 0){
			printf("Failed to get buffer stride\n");
	}

	// clear screen
	memset(ptr, 0x00, buffer[0]*buffer[1]*4);

	// 0xAARRGGBB
	// line 0 to 1/4:	0xFFFF0000
	// line 1/4 to 2/4:	0xFF00FF00
	// line 2/4 to 3/4:	0x000000FF
	// line 3/4 to 1:	0x660000FF
	int one_quarter = buffer[1]/4;
	for(int i = 0; i < buffer[1]; ++i, ptr+=(stride/4)){
		for(int j = 0; j < buffer[0]; j++ ){
			if (i < one_quarter)
				ptr[j] = 0xFFFF0000;
			else if (i < one_quarter*2)
				ptr[j] = 0xFF00FF00;
			else if (i < one_quarter*3)
				ptr[j] = 0x000000FF;
			else
				ptr[j] = 0x660000FF;
		}
	}

	err = screen_post_window(screen_window, screen_buf, 0, NULL, 0);
	if(err != 0){
			printf("Failed to post window\n");
	}

	/* Trap execution */
	while(1);

	screen_destroy_window(screen_window);
	screen_destroy_context(screen_context);

	return EXIT_SUCCESS;
}

