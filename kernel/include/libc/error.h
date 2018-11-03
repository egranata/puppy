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

#ifndef LIBC_ERROR
#define LIBC_ERROR

#include <kernel/sys/stdint.h>
#include <kernel/sys/nocopy.h>
#include <kernel/panic/panic.h>

template<typename result_t>
class ResultOrError {
    public:
        using err_t = const char*;

        static ResultOrError error(const err_t& e) {
            return ResultOrError(e);
        }
        static ResultOrError result(const result_t& r) {
            return ResultOrError(r, true);
        }

        bool checkError(err_t* e) {
            if (mIsError) {
                if (e) {
                    *e = mError;
                    mIsConsumed = true;
                }
                return true;
            }
            return false;
        }

        bool checkResult(result_t* r) {
            if (mIsError) {
                return false;
            }
            *r = mResult;
        }

        result_t resultOrPanic() {
            if (mIsError) {
                if (mError && mError[0]) {
                    PANIC(mError);
                } else {
                    PANIC("failure in protected operation");
                }
            }
            return mResult;
        }

        ResultOrError& operator=(const ResultOrError&) = delete;
        ResultOrError(const ResultOrError&) = delete;

        ResultOrError(ResultOrError&& other) {
            mIsConsumed = false;

            if (other.checkError(&mError)) {
                mIsError = true;
            } else {
                other.checkResult(&mResult);
                mIsError = false;
            }
        }

        ~ResultOrError() {
            if (mIsError && !mIsConsumed) {
                PANIC("required error checking not performed!");
            }
        }

    private:
        // there is probably some fancy C++ trick one can use here, but in case
        // result_t == err_t (or there is some conversion between them), put an
        // extra argument on one of the constructors to tell which is which - the helper
        // static members above will deal with this so callers don't have to
        ResultOrError(const result_t& r, bool) {
            mResult = r;
            mIsError = false;
            mIsConsumed = false;
        }

        ResultOrError(const err_t& e) {
            mError = e;
            mIsError = true;
            mIsConsumed = false;
        }

        struct {
            union {
                result_t mResult;
                err_t mError;
            };
            struct {
                bool mIsError;
                bool mIsConsumed;
            };
        };
};

#endif
