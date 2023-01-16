#include <png.h>
#include <stdio.h>


int draw_png(int *disp_buffer, int disp_width, int disp_height, int stride,
	char *image_path, int x_offset, int y_offset, int visible)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep* row_pointers;

	int width = 0;
	int height = 0;

	int x, y, temp, color_type;
	int *ptr_tmp = disp_buffer;

	fp = fopen(image_path, "rb");
	if (fp == NULL) {
		printf("load_png_image err:fp == NULL");
		return -1;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info_ptr = png_create_info_struct(png_ptr);

	setjmp(png_jmpbuf(png_ptr));

	png_init_io(png_ptr, fp);
	/* 读取PNG图片信息和像素数据 */
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
	/* 获取图像的色彩类型 */
	color_type = png_get_color_type(png_ptr, info_ptr);

	/* 获取图像的宽高 */
	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);

	/* 获取图像的所有行像素数据，row_pointers里边就是rgba数据 */
	row_pointers = png_get_rows(png_ptr, info_ptr);


	ptr_tmp = ptr_tmp + (stride/4) * y_offset;

	/* 根据不同的色彩类型进行相应处理 */
	switch (color_type) {
	case PNG_COLOR_TYPE_RGB_ALPHA:
		for (y = 0; y<disp_height-y_offset && y<height; ptr_tmp+=(stride/4), ++y)
		{
			for (x = 0; x<width && x<disp_width-x_offset; ++x)
			{
				if (visible)
					// 0xRRGGBBAA ---> 0xAARRGGBB
					ptr_tmp[x + x_offset] = (row_pointers[y][4 * x + 3] << 24) |
								(row_pointers[y][4 * x + 0] << 16) |
								(row_pointers[y][4 * x + 1] << 8) |
								 row_pointers[y][4 * x + 2];
				else
					ptr_tmp[x + x_offset] = 0x00000000;
			}
		}
		break;
	case PNG_COLOR_TYPE_RGB:
		for (y = 0; y<height && y<disp_height-y_offset; ptr_tmp+=(stride/4), ++y)
		{
			for (x = 0; x<width && x<disp_width-x_offset; ++x)
			{
				if (visible)
					// 0xRRGGBB ---> 0xAARRGGBB
					ptr_tmp[x + x_offset] = (0xFF << 24) |
								(row_pointers[y][3 * x + 0] << 16) |
								(row_pointers[y][3 * x + 1] << 8) |
								 row_pointers[y][3 * x + 2];
				else
					ptr_tmp[x + x_offset] = 0x00000000;
			}
		}
		break;
	default:
		/* 其它色彩类型的图像就不读了 */
		break;
	}

	fclose(fp);
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	return 0;
}