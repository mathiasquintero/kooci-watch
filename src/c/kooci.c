#include <pebble.h>

bool ready_to_send = false;

static Window *s_window;
static TextLayer *s_text_layer;

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Select");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Up");
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Down");
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(s_text_layer, "Press a button");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  // Read sample 0's x, y, and z values
  uint16_t x = data[0].x;
  uint16_t y = data[0].y;
  uint16_t z= data[0].z;
  bool did_vibrate = data[0].did_vibrate;
  uint64_t timestamp = data[0].timestamp;

  if(did_vibrate || !ready_to_send)
  {
    return;
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "t: %llu, x: %d, y: %d, z: %d",
                                                        timestamp, x, y, z);

  int16_t key1= 0;
  int16_t key2= 1;

  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;


  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);

  if(result == APP_MSG_OK ) {
  // Construct the message
    APP_LOG(APP_LOG_LEVEL_INFO,"----- NO ERROR------");

    // A dummy value

  int value =0;
  //dict_write_int(out_iter,3, &value, sizeof(int), true);
 // Add an item to ask for weather data
  dict_write_uint16(out_iter, 0, x);
  dict_write_uint16(out_iter, 1, y);
  dict_write_uint16(out_iter, 2, z);

  result = app_message_outbox_send();

    // Check the result
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
    else{
      APP_LOG(APP_LOG_LEVEL_INFO, "Sent the message succesfully");
      ready_to_send = false;
    }
  } else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}


static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message succesfully received by MATHIASSSSSSSS");
  ready_to_send = true;
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  // A new message has been successfully received
 APP_LOG(APP_LOG_LEVEL_INFO, "Message succesfully received from MATHIASSSSSSSS");
 ready_to_send = true;
}

int main(void) {
  prv_init();

  // Open AppMessage
  const int inbox_size = 1024;
  const int outbox_size = 1024;
  app_message_open(inbox_size, outbox_size);
  accel_data_service_subscribe(10, accel_data_handler);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_inbox_received(inbox_received_callback);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
