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

#include "../include/getline.h"
#include "../include/document.h"
#include "../include/commands.h"
#include <newlib/stdlib.h>
#include <newlib/stdio.h>

namespace {
    bool append(Document& doc) {
        while(true) {
            auto line = getline("line: ");
            if (line == ".") return true;
            doc.insert(line);
            printf("%u: %s\n", doc.currentLine(), line.c_str());
        }
    }

    bool dump(Document& doc) {
        auto text = doc.toString();
        printf("\n%s\n", text.c_str());
        return true;
    }

    bool list(Document& doc) {
        auto i = 0u;
        for (const auto& line : doc.toLines()) {
            if (i == doc.currentLine()) printf(">> ");
            else                        printf("   ");
            printf("%u: %s\n", i+1, line.c_str());
            ++i;
        }
        if (i == doc.currentLine()) printf(">> current line appends at end\n");
        return true;
    }

    bool position(Document& doc) {
        printf("new line number: ");
        size_t s = doc.currentLine();
        auto ok = scanf("%u%*1[\n]", &s);
        if (ok > 0) {
            doc.move(s-1);
            return true;
        } else return false;
    }

    bool save(Document& doc) {
        auto text = doc.toString();
        auto where = getline("dest file: ");
        FILE* f = fopen(where.c_str(), "w");
        if (f == nullptr) return false;
        fwrite(text.c_str(), 1, text.size(), f);
        fclose(f);

        return true;
    }
}

void loadCommands(Commands& cmds) {
    cmds.addCommand("append", append);
    cmds.addCommand("dump", dump);
    cmds.addCommand("list", list);
    cmds.addCommand("save", save);
    cmds.addCommand("position", position);
}

int main() {
    Document theDoc;
    Commands theCommands;

    loadCommands(theCommands);

    while(true) {
        auto cmd = getline();
        if (theCommands.handleCommand(cmd, theDoc)) continue;
        printf("error: '%s' unknown or unhandled\n", cmd.c_str());
    }
}
