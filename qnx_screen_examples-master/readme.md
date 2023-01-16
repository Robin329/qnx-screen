Build:

	ntoaarch64-gcc -std=c11 example1.c -o example1 -lscreen
	ntoaarch64-gcc -std=c11 example2.c -o example2 -lscreen -lEGL -lGLESv2

References:

	http://www.qnx.com/developers/docs/7.0.0
	http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.screen/topic/manual/cscreen_about.html

	https://github.com/kyo504/gles-tutorials
	https://github.com/ManuelRodM/QNX_hello_window
	https://github.com/ManuelRodM/QNX_hello_triangle
