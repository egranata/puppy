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

#if defined(__linux__)
#error "__linux__ defined - not building with a cross compiler?"
#endif
 
#if !defined(__i386__)
#error "__i386__ not defined - building with a 64-bit compiler?"
#endif

#if !defined(__puppy__)
#error "__puppy__ not defined - not building with a cross compiler?"
#endif
