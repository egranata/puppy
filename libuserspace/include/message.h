/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBUSERSPACE_MESSAGE
#define LIBUSERSPACE_MESSAGE

#include <stdint.h>
#include <kernel/syscalls/types.h>

struct message : public message_t {
    public:
        message() = default;
        
        static message receive();
        static bool receive(message_t*);

        static void send(uint16_t dest, uint32_t a1, uint32_t a2);
};

#endif
