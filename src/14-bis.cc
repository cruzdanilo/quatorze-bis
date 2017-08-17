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
#include <math.h>

#include <libretro.h>
#include <encodings/utf.h>
#include <glsym/glsym.h>
#include <v8.h>
#include <libplatform/libplatform.h>

#include "base/global.h"
#include "base/console.h"

static v8::Platform* platform;
static v8::Isolate* isolate;
static v8::ArrayBuffer::Allocator* allocator;
static v8::Persistent<v8::Context> persistent_context;
static unsigned width  = 640;
static unsigned height = 480;
static struct retro_hw_render_callback hw_render;

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_init(void) {
  quatorzebis::Console::Initialize(environ_cb);

  v8::V8::InitializeICUDefaultLocation(".");
  v8::V8::InitializeExternalStartupData(".");
  platform = v8::platform::CreateDefaultPlatform();
  v8::V8::InitializePlatform(platform);
  v8::V8::Initialize();
  allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = allocator;
  isolate = v8::Isolate::New(params);
}

void retro_deinit(void) {
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete platform;
  delete allocator;
}

unsigned retro_api_version(void) { return RETRO_API_VERSION; }

void retro_set_controller_port_device(unsigned port, unsigned device) {
  (void)port;
  (void)device;
}

void retro_get_system_info(struct retro_system_info *info) {
  memset(info, 0, sizeof(*info));
  info->library_name = "14-bis";
  info->library_version = "v1";
  info->need_fullpath = false;
  info->valid_extensions = "js";
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
  info->timing = (struct retro_system_timing) {
    .fps = 60.0,
    .sample_rate = 30000.0,
  };

  info->geometry = (struct retro_game_geometry) {
    .base_width = width,
    .base_height = height,
    .max_width = width,
    .max_height = height,
    .aspect_ratio = static_cast<float>(width) / height,
  };
}

void retro_set_environment(retro_environment_t cb) {
  environ_cb = cb;
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

  glBindFramebuffer(GL_FRAMEBUFFER, hw_render.get_current_framebuffer());
  glClearColor(0.3, 0.4, 0.5, 1.0);
  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  video_cb(RETRO_HW_FRAME_BUFFER_VALID, width, height, 0);
}

static void context_reset(void) {
  quatorzebis::Console::Log("context reset\n");
  rglgen_resolve_symbols(hw_render.get_proc_address);
}

static void context_destroy(void) {
  quatorzebis::Console::Log("context destroy\n");
}

bool retro_load_game(const struct retro_game_info *info) {
  enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
  if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
    quatorzebis::Console::Log("xrgb8888 is not supported.\n");
    return false;
  }

  hw_render.context_type = RETRO_HW_CONTEXT_OPENGL;
  hw_render.context_reset = context_reset;
  hw_render.context_destroy = context_destroy;
  hw_render.depth = true;
  hw_render.stencil = true;
  hw_render.bottom_left_origin = true;
  if (!environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render)) {
    quatorzebis::Console::Log("opengl is not supported\n");
    return false;
  }

  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = v8::Context::New(isolate, NULL,
      quatorzebis::global::CreateTemplate(isolate));
  if (context.IsEmpty()) {
    quatorzebis::Console::Log("error creating v8 context\n");
    return false;
  }
  persistent_context.Reset(isolate, context);

  v8::TryCatch try_catch(isolate);
  v8::Context::Scope context_scope(context);
  v8::Local<v8::String> source =
      v8::String::NewFromUtf8(isolate,
                              reinterpret_cast<const char *>(info->data),
                              v8::NewStringType::kNormal,
                              info->size).ToLocalChecked();
  v8::Local<v8::Script> script;
  v8::Local<v8::Value> result;
  if (!v8::Script::Compile(context, source).ToLocal(&script)) {
    result = try_catch.Exception();
  } else {
    if (!script->Run(context).ToLocal(&result)) {
      result = try_catch.Exception();
    }
  }
  v8::String::Value utf16(result);
  char *cstr = new char[utf16.length() + 1];
  utf16_to_char_string(*utf16, cstr, utf16.length() + 1);
  quatorzebis::Console::Log(cstr);
  delete[] cstr;

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
