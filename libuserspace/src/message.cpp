// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <libuserspace/syscalls.h>
#include <libuserspace/message.h>

void message_t::send() {
    msgsend_syscall(sender, a1, a2);
}
void message_t::receive(bool wait) {
    msgrecv_syscall((uint32_t)this, wait ? 1 : 0);
}
