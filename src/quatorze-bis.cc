// 14-bis (quatorze-bis)
// Copyright (C) 2017 Danilo Neves Cruz
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <string.h>

#include <libretro.h>
#include <encodings/utf.h>
#include <v8.h>
#include <libplatform/libplatform.h>

static v8::Platform* platform;
static v8::Isolate::CreateParams create_params;
static v8::Isolate* isolate;
static v8::Persistent<v8::Context> persistent_context;
static uint32_t *frame_buf;

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_log_printf_t log_cb;
static void log_null(enum retro_log_level level, const char *fmt, ...) {}

void retro_init(void) {
  struct retro_log_callback log;
  if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
     log_cb = log.log;
  else
     log_cb = log_null;

  v8::V8::InitializeICUDefaultLocation(".");
  v8::V8::InitializeExternalStartupData(".");
  platform = v8::platform::CreateDefaultPlatform();
  v8::V8::InitializePlatform(platform);
  v8::V8::Initialize();
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  isolate = v8::Isolate::New(create_params);

  frame_buf = new uint32_t[640 * 480]();
}

void retro_deinit(void) {
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete platform;
  delete create_params.array_buffer_allocator;

  delete[] frame_buf;
}

unsigned retro_api_version(void) { return RETRO_API_VERSION; }

void retro_set_controller_port_device(unsigned port, unsigned device) {
  log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info) {
  memset(info, 0, sizeof(*info));
  info->library_name = "14-bis (quatorze-bis)";
  info->library_version = "v1";
  info->need_fullpath = false;
  info->valid_extensions = NULL;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
  info->timing = (struct retro_system_timing) {
    .fps = 60.0,
    .sample_rate = 30000.0,
  };

  info->geometry = (struct retro_game_geometry) {
    .base_width = 640,
    .base_height = 480,
    .max_width = 640,
    .max_height = 480,
    .aspect_ratio = 4.0 / 3.0,
  };
}

void retro_set_environment(retro_environment_t cb) {
  environ_cb = cb;

  bool no_rom = true;
  cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);
}

void retro_set_audio_sample(retro_audio_sample_t cb) {
  audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
  audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb) {
  input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb) {
  input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb) {
  video_cb = cb;
}

void retro_run(void) {
  input_poll_cb();

  if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP)) {
    v8::Isolate::Scope isolate_scope(isolate);
    // create a stack-allocated handle scope
    v8::HandleScope handle_scope(isolate);
    // create a new context
    v8::Local<v8::Context> context = persistent_context.Get(isolate);
    // enter the context for compiling and running the hello world script
    v8::Context::Scope context_scope(context);
    // create a string containing the javascript source code
    v8::Local<v8::String> source =
        v8::String::NewFromUtf8(isolate, "'hello' + ', world!'",
                                v8::NewStringType::kNormal).ToLocalChecked();
    // compile the source code
    v8::Local<v8::Script> script =
        v8::Script::Compile(context, source).ToLocalChecked();
    // run the script to get the result
    v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
    // convert the result to an ascii c string and print it
    v8::String::Value utf16(result);
    char *cstr = new char[utf16.length() + 1];
    utf16_to_char_string(*utf16, cstr, utf16.length() + 1);
    log_cb(RETRO_LOG_INFO, "%s\n", cstr);
    delete[] cstr;
  }

  video_cb(frame_buf, 640, 480, sizeof(uint32_t) * 640);
}

bool retro_load_game(const struct retro_game_info *info) {
  v8::Isolate::Scope isolate_scope(isolate);
  // create a stack-allocated handle scope
  v8::HandleScope handle_scope(isolate);
  // create a new context
  v8::Local<v8::Context> context = v8::Context::New(isolate);
  persistent_context.Reset(isolate, context);
  log_cb(RETRO_LOG_INFO, "Loaded game!\n");
  (void)info;
  return true;
}

void retro_unload_game(void) {
  persistent_context.Reset();
}

unsigned retro_get_region(void) {
  return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info,
                             size_t num) {
  (void)type;
  (void)info;
  (void)num;
  return false;
}

size_t retro_serialize_size(void) {
  return 0;
}

bool retro_serialize(void *data, size_t size) {
  (void)data;
  (void)size;
  return false;
}

bool retro_unserialize(const void *data, size_t size) {
  (void)data;
  (void)size;
  return false;
}

void *retro_get_memory_data(unsigned id) {
  (void)id;
  return NULL;
}

size_t retro_get_memory_size(unsigned id) {
  (void)id;
  return 0;
}

void retro_reset(void) {}

void retro_cheat_reset(void) {}

void retro_cheat_set(unsigned index, bool enabled, const char *code) {
  (void)index;
  (void)enabled;
  (void)code;
}
