

#include <screen/screen.h>
#include <iostream>

int main() {
  screen_context_t screen_pctx;
  screen_create_context(&screen_pctx, SCREEN_APPLICATION_CONTEXT);
  /* Create the producer's stream */
  screen_stream_t stream_p;
  auto success = screen_create_stream(&stream_p, screen_pctx);
  if (success == -1) {
    std::cout << "failed to create stream \n";
    return -1;
  }

  int buffer_size[2] = {720, 720};
  screen_set_stream_property_iv(stream_p, SCREEN_PROPERTY_BUFFER_SIZE,
                                buffer_size);
  screen_set_stream_property_iv(stream_p, SCREEN_PROPERTY_FORMAT,
                                (const int[]){SCREEN_FORMAT_RGBA8888});
  screen_set_stream_property_iv(
      stream_p, SCREEN_PROPERTY_USAGE,
      (const int[]){SCREEN_USAGE_OPENGL_ES2 | SCREEN_USAGE_WRITE |
                    SCREEN_USAGE_NATIVE});

  success = screen_create_stream_buffers(stream_p, 2);
  if (success == -1) {
    std::cout << "failed to create stream buffer\n";
    return -1;
  }

  int permissions;
  screen_get_stream_property_iv(stream_p, SCREEN_PROPERTY_PERMISSIONS,
                                &permissions);
  /* Allow processes in the same group to access the stream */
  permissions |= SCREEN_PERMISSION_IROTH;
  screen_set_stream_property_iv(stream_p, SCREEN_PROPERTY_PERMISSIONS,
                                &permissions);

  screen_buffer_t stream_buf = nullptr;
  while (1) {
    success = screen_get_stream_property_pv(  // buffers property
        /* A handle to the buffer or buffers available for rendering. */
        stream_p, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&stream_buf);

    if (success == -1) {
      std::cout << "failed to get stream buffer\n";
      return -1;
    }

    if (stream_buf == nullptr) {
      std::cout << "failed to get stream buffer form stream\n";
      return -1;
    }

    void *pointer;
    screen_get_buffer_property_pv(stream_buf, SCREEN_PROPERTY_POINTER,
                                  &pointer);

    for (int i = 0; i < 720; i++) {
      for (int j = 0; j < 720; j++) {
        ((char *)pointer)[(i * 720 + j) * 4 + 2] += 5;
      }
    }
    //*(char *)pointer = 0xff;

    success = screen_post_stream(stream_p, stream_buf, 0, /* lect cnt is zero */
                                 nullptr, 0);
    if (success == -1) {
      std::cout << "failed to post stream\n";
    }
  }
}
