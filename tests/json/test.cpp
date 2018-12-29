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

#include <checkup/test.h>
#include <checkup/assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <parson/parson.h>

class TheTest : public Test {
    public:
        TheTest() : Test("json") {}
    
    protected:
        void run() override {
            const char* input = "{"
            "\"key1\" : [0.25,1.5,1.875],"
            "\"key2\" : \"value\","
            "\"key3\" : 0.25"
            "}";

            JSON_Value* jv = json_parse_string(input);
            CHECK_NOT_EQ(jv, nullptr);

            CHECK_EQ(json_value_get_type(jv), JSONObject);

            JSON_Object* root_object = json_value_get_object(jv);
            CHECK_NOT_EQ(root_object, nullptr);

            JSON_Array* array = json_object_get_array(root_object, "key1");
            CHECK_NOT_EQ(array, nullptr);
            CHECK_EQ(3, json_array_get_count(array));

            // TODO: floating point equality is a bad bad bad idea
            CHECK_EQ(json_array_get_number(array, 0), 0.25);
            CHECK_EQ(json_array_get_number(array, 1), 1.5);
            CHECK_EQ(json_array_get_number(array, 2), 1.875);

            const char* key2 = json_object_get_string(root_object, "key2");
            CHECK_NOT_EQ(key2, nullptr);
            CHECK_EQ(0, strcmp(key2, "value"));

            double key3 = json_object_get_number(root_object, "key3");
            CHECK_EQ(key3, 0.25);
        }
};

int main() {
    Test* test = new TheTest();
    test->test();
    return 0;
}
