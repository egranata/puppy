// Copyright 2019 Google LLC
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

// prototype for the kernel_result_t type

#include <stdio.h>
#include <stdlib.h>
#include <type_traits>

#define KERNEL_ERROR_CODE(A,B) A = B,
enum class kernel_status_t {
    #include "errors.tbl"
};

#define KERNEL_ERROR_CODE(A,B) if (err == kernel_status_t:: A) return #A;
const char *status_to_string(kernel_status_t err) {
    #include "errors.tbl"
    return "unknown";
}

void panic(const char* s) {
    printf("%s\n", s);
    abort();
}

template<typename T>
struct kernel_result_value_t {
    kernel_result_value_t(const T& value, kernel_status_t) {
        success = value;
    }
    kernel_result_value_t(kernel_status_t e) {
        if (e == kernel_status_t::SUCCESS) panic("constructing a failure result with SUCCESS status");
        status = e;
    }

    union {
        T success;
        kernel_status_t status;
    };
};

template<>
struct kernel_result_value_t<void> {
    kernel_result_value_t(kernel_status_t e) : status(e) {}

    kernel_status_t status;
};

template<typename T>
struct is_not_void : public std::negation<std::is_void<T>> {};

template<typename T>
class kernel_result_t {
    public:
        explicit operator bool() const {
            return mOk == true;
        }

        template<typename R = T, typename Q = std::enable_if<std::is_void<R>::value>>
        static kernel_result_t result(kernel_status_t e) {
            return kernel_result_t(e);
        }

        template<typename R = T, typename Q = std::enable_if<std::is_void<R>::value>>
        static kernel_result_t success() {
            return kernel_result_t(kernel_status_t::SUCCESS);
        }

        template<typename R = T, typename Q = std::enable_if<is_not_void<R>::value>>
        static kernel_result_t success(R arg) {
            return kernel_result_t(arg, kernel_status_t::SUCCESS);
        }

        static kernel_result_t failure(kernel_status_t e) {
            return kernel_result_t(e);
        }

        bool ok() {
            return mChecked = true, mOk;
        }

        template<typename R = T, typename Q = std::enable_if<is_not_void<R>::value>>
        R result() {
            if (ok()) return mResult.success;
            else panic("result extracted out of failure result");
        }

        kernel_status_t status() {
            if (ok()) return kernel_status_t::SUCCESS;
            return mResult.status;
        }

        ~kernel_result_t() {
            if (!mChecked) panic("result not checked before destruction");
        }

    private:
        kernel_result_t(const kernel_result_t&) = delete;
        kernel_result_t(const kernel_result_t&&) = delete;

        template<typename Y>
        kernel_result_t<T>& operator=(const kernel_result_t<Y>&) = delete;
        template<typename Y>
        kernel_result_t<T>& operator=(const kernel_result_t<Y>&&) = delete;

        kernel_result_t(kernel_status_t e) : mOk(e == kernel_status_t::SUCCESS), mResult(e) {}

        template<typename R = T, typename Q = std::enable_if<is_not_void<R>::value>>
        kernel_result_t(R arg, kernel_status_t) : mOk(true), mResult(arg, kernel_status_t::SUCCESS) {}

        bool mOk;
        bool mChecked = false;
        kernel_result_value_t<T> mResult;
};

int main() {
    kernel_result_t<void> void_ok(kernel_result_t<void>::success());
    printf("%s\n", status_to_string(void_ok.status()));

    kernel_result_t<void> void_fail(kernel_result_t<void>::failure(kernel_status_t::NO_SUCH_OBJECT));
    printf("%s\n", status_to_string(void_fail.status()));

    kernel_result_t<int> int_ok(kernel_result_t<int>::success(123));
    printf("%s\n", status_to_string(int_ok.status()));
    if (int_ok.ok()) printf("    %d\n", int_ok.result());

    kernel_result_t<int> int_fail(kernel_result_t<int>::failure(kernel_status_t::INVALID_PARAMETER));
    printf("%s\n", status_to_string(int_fail.status()));
    if (int_fail.ok()) printf("    %d\n", int_fail.result());

    return 0;
}
