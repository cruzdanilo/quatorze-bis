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

#ifndef BASE_GLOBAL_H_
#define BASE_GLOBAL_H_

#include <v8.h>

namespace quatorzebis {
namespace global {
v8::Local<v8::ObjectTemplate> CreateTemplate(v8::Isolate *isolate);
}  // namespace global
}  // namespace quatorzebis

#endif  // BASE_GLOBAL_H_
