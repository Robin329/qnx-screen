
#include <screen/screen.h>
#include <iostream>
#include <thread>

auto create_window(screen_context_t screen_ctx) -> screen_window_t {
  screen_window_t screen_win;

  screen_create_window(&screen_win, screen_ctx);

  screen_set_window_property_iv(
      screen_win, SCREEN_PROPERTY_USAGE,
      (const int[]){SCREEN_USAGE_OPENGL_ES2 | SCREEN_USAGE_WRITE |
                    SCREEN_USAGE_NATIVE});

  int buffer_size[2] = {720, 720};
  screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE,
                                buffer_size);

  screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT,
                                (const int[]){SCREEN_FORMAT_RGBA8888});
  int nbuffers = 2;
  screen_create_window_buffers(screen_win, nbuffers);

  return screen_win;
}

auto get_render_buffer(screen_window_t screen_win) -> screen_buffer_t {
  screen_buffer_t screen_wbuf[2] = {0};
  screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_RENDER_BUFFERS,
                                (void **)&screen_wbuf);
  return screen_wbuf[0];
}

int main() {
  std::cout << "consumer\n";

  screen_context_t screen_ctx;
  screen_create_context(&screen_ctx, SCREEN_APPLICATION_CONTEXT);

  screen_stream_t stream_c;
  screen_create_stream(&stream_c, screen_ctx);

  // create a context
  screen_window_t screen_win = create_window(screen_ctx);

  // get producers' stream
  screen_event_t event;
  int stream_p_id = -1;
  screen_stream_t stream_p = NULL;
  screen_buffer_t
      acquired_buffer; /* Buffer that's been acquired from a stream */

  /* Create an event so that you can retrieve an event from Screen. */
  screen_create_event(&event);

  std::thread renderer;

  while (1) {
    std::cout << "consumer) waiting evetns\n";
    int event_type = SCREEN_EVENT_NONE;
    int object_type;

    /* Get event from Screen for the consumer's context. */
    screen_get_event(screen_ctx, event, -1);

    /* Get the type of event from the event retrieved. */
    screen_get_event_property_iv(event, SCREEN_PROPERTY_TYPE, &event_type);

    /* Process the event if it's a SCREEN_EVENT_CREATE event. */
    if (event_type == SCREEN_EVENT_CREATE) {
      std::cout << "consumer) SCREEN_EVENT_CREATE\n";
      /* Determine that this event is due to a producer stream granting
       * permissions. */
      screen_get_event_property_iv(event, SCREEN_PROPERTY_OBJECT_TYPE,
                                   &object_type);

      if (object_type == SCREEN_OBJECT_TYPE_STREAM) {
        /* Get the handle for the producer's stream from the event. */
        screen_get_event_property_pv(event, SCREEN_PROPERTY_STREAM,
                                     (void **)&stream_p);

        if (stream_p != NULL) {
          /* Get the handle for the producer's stream ID from the event.
           * If there are multiple producers in the system, consumers can use
           * the producer's stream ID
           * as a way to verify whether the SCREEN_EVENT_CREATE event is from
           * the producer that the consumer
           * is expecting. In this example, we assume there's only one producer
           * in the system.
           */
          screen_get_stream_property_iv(stream_p, SCREEN_PROPERTY_ID,
                                        &stream_p_id);

          std::cout << "consumer) found producer : " << stream_p_id << "\n";

          // This function is used when the consumer of a stream is in a
          // different process. Establish the connection between the consumer's
          // stream and the producer's stream
          auto success =
              screen_consume_stream_buffers(stream_c,  // consumer
                                            0,  // num of the buffers shared
                                            stream_p);  // producer
          if (success == -1) {
            std::cout << " failed to consume the stream\n";
            return -1;
          }

          renderer = std::thread([&]() {
            screen_buffer_t sbuffer = nullptr;
            while (1) {
              // blocks until there's a front buffer available to acquire.
              // if don't block, SCREEN_ACQUIRE_DONT_BLOCK
              success = screen_acquire_buffer(&sbuffer, stream_c, nullptr,
                                              nullptr, nullptr, 0);
              if (success == -1) {
                std::cout << "consumer) failed to acquired_buffer\n";
                break;
              }

              // get window buffer
              auto wbuffer = get_render_buffer(screen_win);
              if (wbuffer != nullptr) {
                screen_blit(screen_ctx, wbuffer, sbuffer, nullptr);
                screen_post_window(screen_win, wbuffer, 0, nullptr, 0);
              }

              if (sbuffer != nullptr) {
                screen_release_buffer(sbuffer);
              }
            }
            std::cout << "consumer) finish to render !\n";
          });
        }
      }
    }
    if (event_type == SCREEN_EVENT_CLOSE) {
      std::cout << "consumer) SCREEN_EVENT_CLOSE\n";
      /* Determine that this event is due to a producer stream denying
       * permissions. */
      screen_get_event_property_iv(event, SCREEN_PROPERTY_OBJECT_TYPE,
                                   &object_type);

      if (object_type == SCREEN_OBJECT_TYPE_STREAM) {
        /* Get the handle for the producer's stream from the event. */
        screen_get_event_property_pv(event, SCREEN_PROPERTY_STREAM,
                                     (void **)&stream_p);

        if (stream_p != NULL) {
          /* Get the handle for the producer's stream ID from the event.
           * If there are multiple producers in the system, consumers can use
           * the producer's stream ID
           * as a way to verify whether the SCREEN_EVENT_CREATE event is from
           * the producer that the consumer
           * is expecting. In this example, we assume there's only one producer
           * in the system.
           */
          screen_get_stream_property_iv(stream_p, SCREEN_PROPERTY_ID,
                                        &stream_p_id);
          /* Release any buffers that have been acquired. */
          screen_release_buffer(acquired_buffer);
          /* Deregister asynchronous notifications of updates, if necessary.
             */
          screen_notify(screen_ctx, SCREEN_NOTIFY_UPDATE, stream_p, NULL);
          /* Destroy the consumer stream that's connected to this producer
             stream. */
          screen_destroy_stream(stream_c);
          /* Free up any resources that were locally allocated to track this
             stream. */
          screen_destroy_stream(stream_p);
        }
      }
    }
    // rendering??
  }
  screen_destroy_event(event);
}

