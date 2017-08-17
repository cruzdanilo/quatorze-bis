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

#include "base/console.h"

#include <encodings/utf.h>

static retro_log_printf_t log_cb;
static void log_null(enum retro_log_level level, const char *fmt, ...) {}

namespace quatorzebis {
void Console::Initialize(retro_environment_t environ_cb) {
  struct retro_log_callback log;
  if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
     log_cb = log.log;
  else
     log_cb = log_null;
}

void Console::SetupTemplate(v8::Isolate *isolate,
                            v8::Handle<v8::ObjectTemplate> global) {
  v8::Local<v8::ObjectTemplate> console = v8::ObjectTemplate::New();
  console->Set(v8::String::NewFromUtf8(isolate, "log"),
               v8::FunctionTemplate::New(isolate, Console::Log));
  global->Set(v8::String::NewFromUtf8(isolate, "console"), console);
}

void Console::Log(const v8::FunctionCallbackInfo<v8::Value>& args) {
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    v8::String::Value utf16(args[i]);
    char *cstr = new char[utf16.length() + 1];
    utf16_to_char_string(*utf16, cstr, utf16.length() + 1);
    log_cb(RETRO_LOG_INFO, "%s\n", cstr);
  }
}

void Console::Log(const char *str) {
  log_cb(RETRO_LOG_INFO, str);
}
}  // namespace quatorzebis
