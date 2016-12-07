#include <pebble.h>

#define phoneToWatchUUID = "f5a446f2-ce6e-426f-90f4-f9ffa494b7d3"

static Window *s_window;	
static TextLayer *s_txt_layer;
ActionBarLayer *action_bar;

typedef enum {
  SYSTEM_UNLOCKED,
  SYSTEM_LOCKED,
  SYSTEM_UNKNOWN
} SystemLockState;

// Keys for AppMessage Dictionary
// These should correspond to the values you defined in appinfo.json/Settings
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1
};

// Write message to buffer & send
static void send_message(char msg[]){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_cstring(iter, MESSAGE_KEY, msg);
	
	dict_write_end(iter);
  app_message_outbox_send();
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;

	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", (int)tuple->value->uint32); 
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
	}
  
  
  char msg [] = "I'm a Pebble!";
  send_message(msg);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // A single click has just occured
  
  // When a click occurs, we send an authentication request through the phone to the server
  
  char msg [] = "Start Authentication";
  send_message(msg);
}

void click_config_provider(void *context) {
  
  ButtonId id = BUTTON_ID_SELECT;  // The Select button

  window_single_click_subscribe(id, select_click_handler);  
  //window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) my_next_click_handler);
  //window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) my_previous_click_handler);
}



static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //Create a TextLayer to show the result
  s_txt_layer = text_layer_create(GRect(0, (bounds.size.h/2)-30, bounds.size.w, 60));
  text_layer_set_background_color(s_txt_layer, GColorClear);
  text_layer_set_text_color(s_txt_layer, GColorBlack);
  text_layer_set_font(s_txt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_txt_layer, GTextAlignmentCenter);
  text_layer_set_text(s_txt_layer, "Authenticating");
  layer_add_child(window_layer, text_layer_get_layer(s_txt_layer));
  
  /*
  // Initialize the action bar:
  action_bar = action_bar_layer_create();
  // Associate the action bar with the window:
  action_bar_layer_add_to_window(action_bar, window);
  // Set the click config provider:
  action_bar_layer_set_click_config_provider(action_bar,
                                             click_config_provider);
  */
  // Set the icons:
  // The loading of the icons is omitted for brevity... See gbitmap_create_with_resource()
//  action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, my_icon_previous, true);
//  action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, my_icon_next, true);
}

/*
static void systemlock_toggle_state() {
  DictionaryIterator *out;
  AppMessageResult result = app_message_outbox_begin(&out);
  if (result != APP_MSG_OK) {
    text_layer_set_text(s_txt_layer, "Outbox Failed");
  }

  //dict_write_cstring(out, MESSAGE_KEY_LOCK_UUID, phoneToWatchUUID);
  //dict_write_cstring(out, MESSAGE_KEY_ACCESS_TOKEN, LOCKITRON_ACCESS_TOKEN);

  result = app_message_outbox_send();
  if (result != APP_MSG_OK) {
    text_layer_set_text(s_txt_layer, "Message Failed");
  }
  result = app_message_outbox_send();
}*/

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

static void prv_window_unload(Window *window) {
  window_destroy(s_window);
}

static void init(void) {
	s_window = window_create();
  
  // Use this provider to add button click subscriptions
  window_set_click_config_provider(s_window, click_config_provider);
  
   window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  
	window_stack_push(s_window, false);
	
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
  

  // Initialize AppMessage inbox and outbox buffers with a suitable size
  const int inbox_size = 256;
  const int outbox_size = 256;
	app_message_open(inbox_size, outbox_size);
}

static void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(s_window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}