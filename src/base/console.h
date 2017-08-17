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

#ifndef BASE_CONSOLE_H_
#define BASE_CONSOLE_H_

#include <libretro.h>
#include <v8.h>

namespace quatorzebis {
class Console {
 public:
  static void Initialize(retro_environment_t environ_cb);
  static void SetupTemplate(v8::Isolate *isolate,
                            v8::Handle<v8::ObjectTemplate> global);
  static void Log(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Log(const char *str);
};
}  // namespace quatorzebis

#endif  // BASE_CONSOLE_H_
