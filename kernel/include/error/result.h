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

#ifndef ERROR_RESULT
#define ERROR_RESULT

#include <kernel/error/status.h>
#include <kernel/libc/enableif.h>
#include <kernel/libc/move.h>
#include <kernel/panic/panic.h>

template<typename T>
struct kernel_result_value_t {
    kernel_result_value_t(const T& value, kernel_status_t) {
        success = value;
    }
    kernel_result_value_t(kernel_status_t e) {
        if (e == kernel_status_t::SUCCESS) PANIC("constructing a failure result with SUCCESS status");
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
class kernel_result_t {
    public:
        explicit operator bool() const {
            return mOk == true;
        }

        template<typename R = T, typename Q = enable_if<is_void<R>::value>>
        static kernel_result_t result(kernel_status_t e) {
            return kernel_result_t(e);
        }

        template<typename R = T, typename Q = enable_if<is_void<R>::value>>
        static kernel_result_t success() {
            return kernel_result_t(kernel_status_t::SUCCESS);
        }

        template<typename R = T, typename Q = enable_if<is_not_void<R>::value>>
        static kernel_result_t success(R arg) {
            return kernel_result_t(arg, kernel_status_t::SUCCESS);
        }

        static kernel_result_t failure(kernel_status_t e) {
            return kernel_result_t(e);
        }

        bool ok() {
            return mChecked = true, mOk;
        }
        bool error() {
            return mChecked = true, mOk == false;
        }

        template<typename R = T, typename Q = enable_if<is_not_void<R>::value>>
        R result() {
            if (ok()) return mResult.success;
            else PANIC("result extracted out of failure result");
        }

        template<typename R = T, typename Q = enable_if<is_not_void<R>::value>>
        bool result(R* result) {
            if (ok()) {
                *result = mResult.success;
                return true;
            } else return false;
        }

        kernel_status_t status() {
            if (ok()) return kernel_status_t::SUCCESS;
            return mResult.status;
        }

        ~kernel_result_t() {
            if (!mChecked) PANIC("result not checked before destruction");
        }

        kernel_result_t(const kernel_result_t&& other) :
            mOk(other.mOk),
            mChecked(other.mChecked),
            mResult(move(other.mResult)) {}

    private:
        kernel_result_t(const kernel_result_t&) = delete;

        template<typename Y>
        kernel_result_t<T>& operator=(const kernel_result_t<Y>&) = delete;
        template<typename Y>
        kernel_result_t<T>& operator=(const kernel_result_t<Y>&&) = delete;

        kernel_result_t(kernel_status_t e) : mOk(e == kernel_status_t::SUCCESS), mResult(e) {}

        template<typename R = T, typename Q = enable_if<is_not_void<R>::value>>
        kernel_result_t(R arg, kernel_status_t) : mOk(true), mResult(arg, kernel_status_t::SUCCESS) {}

        bool mOk;
        bool mChecked = false;
        kernel_result_value_t<T> mResult;
};

template<typename T, typename = enable_if<is_not_void<T>::value>>
kernel_result_t<T> kernel_success(T arg) {
    return kernel_result_t<T>::success(arg);
}

template<typename T, typename = enable_if<is_void<T>::value>>
kernel_result_t<T> kernel_success() {
    return kernel_result_t<T>::success();
}

template<typename T>
kernel_result_t<T> kernel_failure(kernel_status_t e) {
    return kernel_result_t<T>::failure(e);
}

#endif
