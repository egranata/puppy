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

int func(int) __attribute__ ((noinline));
int baq(int q) __attribute__ ((noinline));
int baz(int a, int b, int c) __attribute__ ((noinline));
int bar(int x, int y, int z) __attribute__ ((noinline));
int foo(int x) __attribute__ ((noinline));

int func(int) {
    int* p = nullptr;
    return (*p = 2) + 1;
}

int baq(int q) {
    return func(q--);
}

int baz(int a, int b, int c) {
    return baq(a+b-c);
}

int bar(int x, int y, int z) {
    return baz(y, z, x);
}

int foo(int x) {
    return bar(x+1, x-1, 0);    
}

int main(int, char**) {
    return foo(0);
}
