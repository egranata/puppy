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

#ifndef LIBC_PAIR
#define LIBC_PAIR

template<typename T1, typename T2=T1>
class pair {
public:
	T1 first;
	T2 second;
	
	pair(T1 first = T1(), T2 second = T2()) {
		this->first = first;
		this->second = second;
	}
	
	pair(const pair&) = default;
	pair(pair&&) = default;
	pair& operator=(const pair&) = default;
};

#endif
