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

#include <libuserspace/message.h>
#include <libuserspace/printf.h>
#include <libuserspace/getpid.h>
#include <libuserspace/yield.h>

void waitForReceive() {
    printf("[pid %u] Will be waiting for a message\n", getpid());

    message_t msg;
    msg.time = 0;

    while(msg.time == 0) {
        msg.receive(true);
    }

    printf("At time %llu, %u received a message from %u; it says %x %x\n",
        msg.time, getpid(), msg.sender, msg.a1, msg.a2);
}
