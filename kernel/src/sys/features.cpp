/*
 * Copyright 2019 Google LLC
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

#include <kernel/sys/features.h>
#define FEATURE(name, id) static constexpr feature_id_t gFeatureId ## name = id;
#include <kernel/sys/features.tbl>
#undef FEATURE

feature_id_t gEnabledFeatures[] = {
    gFeatureIdPuppy
};

size_t gNumFeatures = sizeof(gEnabledFeatures) / sizeof(gEnabledFeatures[0]);

bool hasFeature(feature_id_t f) {
    if (f == gFeatureIdInvalid) return false;

    for (size_t i = 0; i < gNumFeatures; ++i) {
        if (gEnabledFeatures[i] == f) return true;
    }
    return false;
}
