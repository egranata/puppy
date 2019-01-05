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

#include <kernel/i386/idt.h>
#include <kernel/i386/cpustate.h>
#include <kernel/i386/primitives.h>
#include <kernel/libc/string.h>
#include <kernel/log/log.h>
#include <muzzle/string.h>
#include <kernel/process/manager.h>
#include <kernel/process/current.h>
#include <kernel/synch/waitqueue.h>

LOG_TAG(INIRQ, 2);
LOG_TAG(IRQSETUP, 1);

Interrupts::handler_t::handler_t() : func(nullptr), payload(nullptr), count(0) {
    bzero(name, sizeof(name));
}

Interrupts::handler_t::operator bool() {
    return func != nullptr;
}

static uint32_t gIRQDepthCounter = 0;

extern "C"
void interrupt_handler(GPR gpr, InterruptStack stack) {
    __atomic_add_fetch(&gIRQDepthCounter, 1, __ATOMIC_SEQ_CST);
    bool yield_on_exit = false;

    TAG_DEBUG(INIRQ, "received IRQ %u", stack.irqnumber);
    auto& handler = Interrupts::get().mHandlers[stack.irqnumber];
    handler.count += 1;
    TAG_DEBUG(INIRQ, "IRQ %u occurred %llu times", stack.irqnumber, handler.count);
	if (handler) {
		auto action = handler.func(gpr, stack, handler.payload);
        if ((action & IRQ_RESPONSE_WAKE) == IRQ_RESPONSE_WAKE) {
            if (handler.wq == nullptr) {
                LOG_ERROR("wake requested but IRQ has no waitqueue");
            } else {
                handler.wq->wakeall();
            }
        }
        if ((action & IRQ_RESPONSE_YIELD) == IRQ_RESPONSE_YIELD) {
            if (nullptr == gCurrentProcess) {
                LOG_ERROR("yield requested outside of process context");
            } else {
                yield_on_exit = true;
            }
        }
	} else {
        TAG_DEBUG(INIRQ, "IRQ %u received - no handler", stack.irqnumber);
    }

    __atomic_fetch_sub(&gIRQDepthCounter, 1, __ATOMIC_SEQ_CST);
    if (yield_on_exit) ProcessManager::get().yield();
}

uint32_t Interrupts::irqDepth() const {
    return __atomic_load_n(&gIRQDepthCounter, __ATOMIC_SEQ_CST);
}

Interrupts& Interrupts::get() {
	static Interrupts gInterrupts;
	
	return gInterrupts;
}

uint64_t Interrupts::getNumOccurrences(uint8_t irq) {
    return mHandlers[irq].count;
}

const char* Interrupts::getName(uint8_t irq) {
    return &mHandlers[irq].name[0];
}

static struct {
	uint16_t size;
	uintptr_t offset;
} __attribute__((packed)) gIDT;

static_assert(sizeof(gIDT) == 6, "IDT reference not 6 bytes in size");

void Interrupts::install() {
    gIDT = {sizeof(mEntries)-1, (uintptr_t)&mEntries};
    loadidt((uintptr_t)&gIDT);
}

void Interrupts::enable() {
    if (mCliCount == 0) {
        mCliCount = 1;
        enableirq();
    } else if (mCliCount < 0) {
        ++mCliCount;
    }
}
void Interrupts::disable() {
    if (mCliCount == 1) {
        disableirq();
        mCliCount = 0;
    } else if (mCliCount <= 0) {
        --mCliCount;
    }
}

bool Interrupts::enabled() {
    return 0 != (readflags() & 512);
}

Interrupts::ScopedDisabler::ScopedDisabler() {
    Interrupts::get().disable();
}
Interrupts::ScopedDisabler::~ScopedDisabler() {
    Interrupts::get().enable();
}
Interrupts::ScopedDisabler::operator bool() {
    return false == Interrupts::get().enabled();
}

void Interrupts::sethandler(uint8_t irq, const char* name, handler_t::irq_handler_f f, void* payload, WaitQueue* wq) {
    TAG_INFO(IRQSETUP, "function at 0x%p set as handler for irq %d", f, irq);
    auto& handler = mHandlers[irq];
    handler.payload = payload;
    handler.wq = wq;
    bzero(handler.name, sizeof(handler.name));
    strncpy(handler.name, name, sizeof(handler.name) - 1);
	handler.func = f;
}

void Interrupts::setWakeQueue(uint8_t irq, WaitQueue* wq) {
    auto& handler = mHandlers[irq];
    handler.wq = wq;
}

// generated by tools/make_idt_table.py
extern uintptr_t interrupt_handler_0;
extern uintptr_t interrupt_handler_1;
extern uintptr_t interrupt_handler_2;
extern uintptr_t interrupt_handler_3;
extern uintptr_t interrupt_handler_4;
extern uintptr_t interrupt_handler_5;
extern uintptr_t interrupt_handler_6;
extern uintptr_t interrupt_handler_7;
extern uintptr_t interrupt_handler_8;
extern uintptr_t interrupt_handler_9;
extern uintptr_t interrupt_handler_10;
extern uintptr_t interrupt_handler_11;
extern uintptr_t interrupt_handler_12;
extern uintptr_t interrupt_handler_13;
extern uintptr_t interrupt_handler_14;
extern uintptr_t interrupt_handler_15;
extern uintptr_t interrupt_handler_16;
extern uintptr_t interrupt_handler_17;
extern uintptr_t interrupt_handler_18;
extern uintptr_t interrupt_handler_19;
extern uintptr_t interrupt_handler_20;
extern uintptr_t interrupt_handler_21;
extern uintptr_t interrupt_handler_22;
extern uintptr_t interrupt_handler_23;
extern uintptr_t interrupt_handler_24;
extern uintptr_t interrupt_handler_25;
extern uintptr_t interrupt_handler_26;
extern uintptr_t interrupt_handler_27;
extern uintptr_t interrupt_handler_28;
extern uintptr_t interrupt_handler_29;
extern uintptr_t interrupt_handler_30;
extern uintptr_t interrupt_handler_31;
extern uintptr_t interrupt_handler_32;
extern uintptr_t interrupt_handler_33;
extern uintptr_t interrupt_handler_34;
extern uintptr_t interrupt_handler_35;
extern uintptr_t interrupt_handler_36;
extern uintptr_t interrupt_handler_37;
extern uintptr_t interrupt_handler_38;
extern uintptr_t interrupt_handler_39;
extern uintptr_t interrupt_handler_40;
extern uintptr_t interrupt_handler_41;
extern uintptr_t interrupt_handler_42;
extern uintptr_t interrupt_handler_43;
extern uintptr_t interrupt_handler_44;
extern uintptr_t interrupt_handler_45;
extern uintptr_t interrupt_handler_46;
extern uintptr_t interrupt_handler_47;
extern uintptr_t interrupt_handler_48;
extern uintptr_t interrupt_handler_49;
extern uintptr_t interrupt_handler_50;
extern uintptr_t interrupt_handler_51;
extern uintptr_t interrupt_handler_52;
extern uintptr_t interrupt_handler_53;
extern uintptr_t interrupt_handler_54;
extern uintptr_t interrupt_handler_55;
extern uintptr_t interrupt_handler_56;
extern uintptr_t interrupt_handler_57;
extern uintptr_t interrupt_handler_58;
extern uintptr_t interrupt_handler_59;
extern uintptr_t interrupt_handler_60;
extern uintptr_t interrupt_handler_61;
extern uintptr_t interrupt_handler_62;
extern uintptr_t interrupt_handler_63;
extern uintptr_t interrupt_handler_64;
extern uintptr_t interrupt_handler_65;
extern uintptr_t interrupt_handler_66;
extern uintptr_t interrupt_handler_67;
extern uintptr_t interrupt_handler_68;
extern uintptr_t interrupt_handler_69;
extern uintptr_t interrupt_handler_70;
extern uintptr_t interrupt_handler_71;
extern uintptr_t interrupt_handler_72;
extern uintptr_t interrupt_handler_73;
extern uintptr_t interrupt_handler_74;
extern uintptr_t interrupt_handler_75;
extern uintptr_t interrupt_handler_76;
extern uintptr_t interrupt_handler_77;
extern uintptr_t interrupt_handler_78;
extern uintptr_t interrupt_handler_79;
extern uintptr_t interrupt_handler_80;
extern uintptr_t interrupt_handler_81;
extern uintptr_t interrupt_handler_82;
extern uintptr_t interrupt_handler_83;
extern uintptr_t interrupt_handler_84;
extern uintptr_t interrupt_handler_85;
extern uintptr_t interrupt_handler_86;
extern uintptr_t interrupt_handler_87;
extern uintptr_t interrupt_handler_88;
extern uintptr_t interrupt_handler_89;
extern uintptr_t interrupt_handler_90;
extern uintptr_t interrupt_handler_91;
extern uintptr_t interrupt_handler_92;
extern uintptr_t interrupt_handler_93;
extern uintptr_t interrupt_handler_94;
extern uintptr_t interrupt_handler_95;
extern uintptr_t interrupt_handler_96;
extern uintptr_t interrupt_handler_97;
extern uintptr_t interrupt_handler_98;
extern uintptr_t interrupt_handler_99;
extern uintptr_t interrupt_handler_100;
extern uintptr_t interrupt_handler_101;
extern uintptr_t interrupt_handler_102;
extern uintptr_t interrupt_handler_103;
extern uintptr_t interrupt_handler_104;
extern uintptr_t interrupt_handler_105;
extern uintptr_t interrupt_handler_106;
extern uintptr_t interrupt_handler_107;
extern uintptr_t interrupt_handler_108;
extern uintptr_t interrupt_handler_109;
extern uintptr_t interrupt_handler_110;
extern uintptr_t interrupt_handler_111;
extern uintptr_t interrupt_handler_112;
extern uintptr_t interrupt_handler_113;
extern uintptr_t interrupt_handler_114;
extern uintptr_t interrupt_handler_115;
extern uintptr_t interrupt_handler_116;
extern uintptr_t interrupt_handler_117;
extern uintptr_t interrupt_handler_118;
extern uintptr_t interrupt_handler_119;
extern uintptr_t interrupt_handler_120;
extern uintptr_t interrupt_handler_121;
extern uintptr_t interrupt_handler_122;
extern uintptr_t interrupt_handler_123;
extern uintptr_t interrupt_handler_124;
extern uintptr_t interrupt_handler_125;
extern uintptr_t interrupt_handler_126;
extern uintptr_t interrupt_handler_127;
extern uintptr_t interrupt_handler_128;
extern uintptr_t interrupt_handler_129;
extern uintptr_t interrupt_handler_130;
extern uintptr_t interrupt_handler_131;
extern uintptr_t interrupt_handler_132;
extern uintptr_t interrupt_handler_133;
extern uintptr_t interrupt_handler_134;
extern uintptr_t interrupt_handler_135;
extern uintptr_t interrupt_handler_136;
extern uintptr_t interrupt_handler_137;
extern uintptr_t interrupt_handler_138;
extern uintptr_t interrupt_handler_139;
extern uintptr_t interrupt_handler_140;
extern uintptr_t interrupt_handler_141;
extern uintptr_t interrupt_handler_142;
extern uintptr_t interrupt_handler_143;
extern uintptr_t interrupt_handler_144;
extern uintptr_t interrupt_handler_145;
extern uintptr_t interrupt_handler_146;
extern uintptr_t interrupt_handler_147;
extern uintptr_t interrupt_handler_148;
extern uintptr_t interrupt_handler_149;
extern uintptr_t interrupt_handler_150;
extern uintptr_t interrupt_handler_151;
extern uintptr_t interrupt_handler_152;
extern uintptr_t interrupt_handler_153;
extern uintptr_t interrupt_handler_154;
extern uintptr_t interrupt_handler_155;
extern uintptr_t interrupt_handler_156;
extern uintptr_t interrupt_handler_157;
extern uintptr_t interrupt_handler_158;
extern uintptr_t interrupt_handler_159;
extern uintptr_t interrupt_handler_160;
extern uintptr_t interrupt_handler_161;
extern uintptr_t interrupt_handler_162;
extern uintptr_t interrupt_handler_163;
extern uintptr_t interrupt_handler_164;
extern uintptr_t interrupt_handler_165;
extern uintptr_t interrupt_handler_166;
extern uintptr_t interrupt_handler_167;
extern uintptr_t interrupt_handler_168;
extern uintptr_t interrupt_handler_169;
extern uintptr_t interrupt_handler_170;
extern uintptr_t interrupt_handler_171;
extern uintptr_t interrupt_handler_172;
extern uintptr_t interrupt_handler_173;
extern uintptr_t interrupt_handler_174;
extern uintptr_t interrupt_handler_175;
extern uintptr_t interrupt_handler_176;
extern uintptr_t interrupt_handler_177;
extern uintptr_t interrupt_handler_178;
extern uintptr_t interrupt_handler_179;
extern uintptr_t interrupt_handler_180;
extern uintptr_t interrupt_handler_181;
extern uintptr_t interrupt_handler_182;
extern uintptr_t interrupt_handler_183;
extern uintptr_t interrupt_handler_184;
extern uintptr_t interrupt_handler_185;
extern uintptr_t interrupt_handler_186;
extern uintptr_t interrupt_handler_187;
extern uintptr_t interrupt_handler_188;
extern uintptr_t interrupt_handler_189;
extern uintptr_t interrupt_handler_190;
extern uintptr_t interrupt_handler_191;
extern uintptr_t interrupt_handler_192;
extern uintptr_t interrupt_handler_193;
extern uintptr_t interrupt_handler_194;
extern uintptr_t interrupt_handler_195;
extern uintptr_t interrupt_handler_196;
extern uintptr_t interrupt_handler_197;
extern uintptr_t interrupt_handler_198;
extern uintptr_t interrupt_handler_199;
extern uintptr_t interrupt_handler_200;
extern uintptr_t interrupt_handler_201;
extern uintptr_t interrupt_handler_202;
extern uintptr_t interrupt_handler_203;
extern uintptr_t interrupt_handler_204;
extern uintptr_t interrupt_handler_205;
extern uintptr_t interrupt_handler_206;
extern uintptr_t interrupt_handler_207;
extern uintptr_t interrupt_handler_208;
extern uintptr_t interrupt_handler_209;
extern uintptr_t interrupt_handler_210;
extern uintptr_t interrupt_handler_211;
extern uintptr_t interrupt_handler_212;
extern uintptr_t interrupt_handler_213;
extern uintptr_t interrupt_handler_214;
extern uintptr_t interrupt_handler_215;
extern uintptr_t interrupt_handler_216;
extern uintptr_t interrupt_handler_217;
extern uintptr_t interrupt_handler_218;
extern uintptr_t interrupt_handler_219;
extern uintptr_t interrupt_handler_220;
extern uintptr_t interrupt_handler_221;
extern uintptr_t interrupt_handler_222;
extern uintptr_t interrupt_handler_223;
extern uintptr_t interrupt_handler_224;
extern uintptr_t interrupt_handler_225;
extern uintptr_t interrupt_handler_226;
extern uintptr_t interrupt_handler_227;
extern uintptr_t interrupt_handler_228;
extern uintptr_t interrupt_handler_229;
extern uintptr_t interrupt_handler_230;
extern uintptr_t interrupt_handler_231;
extern uintptr_t interrupt_handler_232;
extern uintptr_t interrupt_handler_233;
extern uintptr_t interrupt_handler_234;
extern uintptr_t interrupt_handler_235;
extern uintptr_t interrupt_handler_236;
extern uintptr_t interrupt_handler_237;
extern uintptr_t interrupt_handler_238;
extern uintptr_t interrupt_handler_239;
extern uintptr_t interrupt_handler_240;
extern uintptr_t interrupt_handler_241;
extern uintptr_t interrupt_handler_242;
extern uintptr_t interrupt_handler_243;
extern uintptr_t interrupt_handler_244;
extern uintptr_t interrupt_handler_245;
extern uintptr_t interrupt_handler_246;
extern uintptr_t interrupt_handler_247;
extern uintptr_t interrupt_handler_248;
extern uintptr_t interrupt_handler_249;
extern uintptr_t interrupt_handler_250;
extern uintptr_t interrupt_handler_251;
extern uintptr_t interrupt_handler_252;
extern uintptr_t interrupt_handler_253;
extern uintptr_t interrupt_handler_254;
extern uintptr_t interrupt_handler_255;

Interrupts::Interrupts() {
    mCliCount = (enabled() ? 1 : 0);
    bzero((uint8_t*)&mHandlers[0], sizeof(mHandlers));
    mEntries[0] = Entry((uintptr_t)&interrupt_handler_0, false, true);
    mEntries[1] = Entry((uintptr_t)&interrupt_handler_1, false, true);
    mEntries[2] = Entry((uintptr_t)&interrupt_handler_2, false, true);
    mEntries[3] = Entry((uintptr_t)&interrupt_handler_3, false, true);
    mEntries[4] = Entry((uintptr_t)&interrupt_handler_4, false, true);
    mEntries[5] = Entry((uintptr_t)&interrupt_handler_5, false, true);
    mEntries[6] = Entry((uintptr_t)&interrupt_handler_6, false, true);
    mEntries[7] = Entry((uintptr_t)&interrupt_handler_7, false, true);
    mEntries[8] = Entry((uintptr_t)&interrupt_handler_8, false, true);
    mEntries[9] = Entry((uintptr_t)&interrupt_handler_9, false, true);
    mEntries[10] = Entry((uintptr_t)&interrupt_handler_10, false, true);
    mEntries[11] = Entry((uintptr_t)&interrupt_handler_11, false, true);
    mEntries[12] = Entry((uintptr_t)&interrupt_handler_12, false, true);
    mEntries[13] = Entry((uintptr_t)&interrupt_handler_13, false, true);
    mEntries[14] = Entry((uintptr_t)&interrupt_handler_14, false, true);
    mEntries[16] = Entry((uintptr_t)&interrupt_handler_16, false, true);
    mEntries[17] = Entry((uintptr_t)&interrupt_handler_17, false, true);
    mEntries[18] = Entry((uintptr_t)&interrupt_handler_18, false, true);
    mEntries[19] = Entry((uintptr_t)&interrupt_handler_19, false, true);
    mEntries[20] = Entry((uintptr_t)&interrupt_handler_20, false, true);
    mEntries[30] = Entry((uintptr_t)&interrupt_handler_30, false, true);
    mEntries[32] = Entry((uintptr_t)&interrupt_handler_32, false, true);
    mEntries[33] = Entry((uintptr_t)&interrupt_handler_33, false, true);
    mEntries[34] = Entry((uintptr_t)&interrupt_handler_34, false, true);
    mEntries[35] = Entry((uintptr_t)&interrupt_handler_35, false, true);
    mEntries[36] = Entry((uintptr_t)&interrupt_handler_36, false, true);
    mEntries[37] = Entry((uintptr_t)&interrupt_handler_37, false, true);
    mEntries[38] = Entry((uintptr_t)&interrupt_handler_38, false, true);
    mEntries[39] = Entry((uintptr_t)&interrupt_handler_39, false, true);
    mEntries[40] = Entry((uintptr_t)&interrupt_handler_40, false, true);
    mEntries[41] = Entry((uintptr_t)&interrupt_handler_41, false, true);
    mEntries[42] = Entry((uintptr_t)&interrupt_handler_42, false, true);
    mEntries[43] = Entry((uintptr_t)&interrupt_handler_43, false, true);
    mEntries[44] = Entry((uintptr_t)&interrupt_handler_44, false, true);
    mEntries[45] = Entry((uintptr_t)&interrupt_handler_45, false, true);
    mEntries[46] = Entry((uintptr_t)&interrupt_handler_46, false, true);
    mEntries[47] = Entry((uintptr_t)&interrupt_handler_47, false, true);
    mEntries[48] = Entry((uintptr_t)&interrupt_handler_48, false, true);
    mEntries[49] = Entry((uintptr_t)&interrupt_handler_49, false, true);
    mEntries[50] = Entry((uintptr_t)&interrupt_handler_50, false, true);
    mEntries[51] = Entry((uintptr_t)&interrupt_handler_51, false, true);
    mEntries[52] = Entry((uintptr_t)&interrupt_handler_52, false, true);
    mEntries[53] = Entry((uintptr_t)&interrupt_handler_53, false, true);
    mEntries[54] = Entry((uintptr_t)&interrupt_handler_54, false, true);
    mEntries[55] = Entry((uintptr_t)&interrupt_handler_55, false, true);
    mEntries[56] = Entry((uintptr_t)&interrupt_handler_56, false, true);
    mEntries[57] = Entry((uintptr_t)&interrupt_handler_57, false, true);
    mEntries[58] = Entry((uintptr_t)&interrupt_handler_58, false, true);
    mEntries[59] = Entry((uintptr_t)&interrupt_handler_59, false, true);
    mEntries[60] = Entry((uintptr_t)&interrupt_handler_60, false, true);
    mEntries[61] = Entry((uintptr_t)&interrupt_handler_61, false, true);
    mEntries[62] = Entry((uintptr_t)&interrupt_handler_62, false, true);
    mEntries[63] = Entry((uintptr_t)&interrupt_handler_63, false, true);
    mEntries[64] = Entry((uintptr_t)&interrupt_handler_64, false, true);
    mEntries[65] = Entry((uintptr_t)&interrupt_handler_65, false, true);
    mEntries[66] = Entry((uintptr_t)&interrupt_handler_66, false, true);
    mEntries[67] = Entry((uintptr_t)&interrupt_handler_67, false, true);
    mEntries[68] = Entry((uintptr_t)&interrupt_handler_68, false, true);
    mEntries[69] = Entry((uintptr_t)&interrupt_handler_69, false, true);
    mEntries[70] = Entry((uintptr_t)&interrupt_handler_70, false, true);
    mEntries[71] = Entry((uintptr_t)&interrupt_handler_71, false, true);
    mEntries[72] = Entry((uintptr_t)&interrupt_handler_72, false, true);
    mEntries[73] = Entry((uintptr_t)&interrupt_handler_73, false, true);
    mEntries[74] = Entry((uintptr_t)&interrupt_handler_74, false, true);
    mEntries[75] = Entry((uintptr_t)&interrupt_handler_75, false, true);
    mEntries[76] = Entry((uintptr_t)&interrupt_handler_76, false, true);
    mEntries[77] = Entry((uintptr_t)&interrupt_handler_77, false, true);
    mEntries[78] = Entry((uintptr_t)&interrupt_handler_78, false, true);
    mEntries[79] = Entry((uintptr_t)&interrupt_handler_79, false, true);
    mEntries[80] = Entry((uintptr_t)&interrupt_handler_80, false, true);
    mEntries[81] = Entry((uintptr_t)&interrupt_handler_81, false, true);
    mEntries[82] = Entry((uintptr_t)&interrupt_handler_82, false, true);
    mEntries[83] = Entry((uintptr_t)&interrupt_handler_83, false, true);
    mEntries[84] = Entry((uintptr_t)&interrupt_handler_84, false, true);
    mEntries[85] = Entry((uintptr_t)&interrupt_handler_85, false, true);
    mEntries[86] = Entry((uintptr_t)&interrupt_handler_86, false, true);
    mEntries[87] = Entry((uintptr_t)&interrupt_handler_87, false, true);
    mEntries[88] = Entry((uintptr_t)&interrupt_handler_88, false, true);
    mEntries[89] = Entry((uintptr_t)&interrupt_handler_89, false, true);
    mEntries[90] = Entry((uintptr_t)&interrupt_handler_90, false, true);
    mEntries[91] = Entry((uintptr_t)&interrupt_handler_91, false, true);
    mEntries[92] = Entry((uintptr_t)&interrupt_handler_92, false, true);
    mEntries[93] = Entry((uintptr_t)&interrupt_handler_93, false, true);
    mEntries[94] = Entry((uintptr_t)&interrupt_handler_94, false, true);
    mEntries[95] = Entry((uintptr_t)&interrupt_handler_95, false, true);
    mEntries[96] = Entry((uintptr_t)&interrupt_handler_96, false, true);
    mEntries[97] = Entry((uintptr_t)&interrupt_handler_97, false, true);
    mEntries[98] = Entry((uintptr_t)&interrupt_handler_98, false, true);
    mEntries[99] = Entry((uintptr_t)&interrupt_handler_99, false, true);
    mEntries[100] = Entry((uintptr_t)&interrupt_handler_100, false, true);
    mEntries[101] = Entry((uintptr_t)&interrupt_handler_101, false, true);
    mEntries[102] = Entry((uintptr_t)&interrupt_handler_102, false, true);
    mEntries[103] = Entry((uintptr_t)&interrupt_handler_103, false, true);
    mEntries[104] = Entry((uintptr_t)&interrupt_handler_104, false, true);
    mEntries[105] = Entry((uintptr_t)&interrupt_handler_105, false, true);
    mEntries[106] = Entry((uintptr_t)&interrupt_handler_106, false, true);
    mEntries[107] = Entry((uintptr_t)&interrupt_handler_107, false, true);
    mEntries[108] = Entry((uintptr_t)&interrupt_handler_108, false, true);
    mEntries[109] = Entry((uintptr_t)&interrupt_handler_109, false, true);
    mEntries[110] = Entry((uintptr_t)&interrupt_handler_110, false, true);
    mEntries[111] = Entry((uintptr_t)&interrupt_handler_111, false, true);
    mEntries[112] = Entry((uintptr_t)&interrupt_handler_112, false, true);
    mEntries[113] = Entry((uintptr_t)&interrupt_handler_113, false, true);
    mEntries[114] = Entry((uintptr_t)&interrupt_handler_114, false, true);
    mEntries[115] = Entry((uintptr_t)&interrupt_handler_115, false, true);
    mEntries[116] = Entry((uintptr_t)&interrupt_handler_116, false, true);
    mEntries[117] = Entry((uintptr_t)&interrupt_handler_117, false, true);
    mEntries[118] = Entry((uintptr_t)&interrupt_handler_118, false, true);
    mEntries[119] = Entry((uintptr_t)&interrupt_handler_119, false, true);
    mEntries[120] = Entry((uintptr_t)&interrupt_handler_120, false, true);
    mEntries[121] = Entry((uintptr_t)&interrupt_handler_121, false, true);
    mEntries[122] = Entry((uintptr_t)&interrupt_handler_122, false, true);
    mEntries[123] = Entry((uintptr_t)&interrupt_handler_123, false, true);
    mEntries[124] = Entry((uintptr_t)&interrupt_handler_124, false, true);
    mEntries[125] = Entry((uintptr_t)&interrupt_handler_125, false, true);
    mEntries[126] = Entry((uintptr_t)&interrupt_handler_126, false, true);
    mEntries[127] = Entry((uintptr_t)&interrupt_handler_127, false, true);
    mEntries[128] = Entry((uintptr_t)&interrupt_handler_128, true, false);
    mEntries[129] = Entry((uintptr_t)&interrupt_handler_129, false, true);
    mEntries[130] = Entry((uintptr_t)&interrupt_handler_130, false, true);
    mEntries[131] = Entry((uintptr_t)&interrupt_handler_131, false, true);
    mEntries[132] = Entry((uintptr_t)&interrupt_handler_132, false, true);
    mEntries[133] = Entry((uintptr_t)&interrupt_handler_133, false, true);
    mEntries[134] = Entry((uintptr_t)&interrupt_handler_134, false, true);
    mEntries[135] = Entry((uintptr_t)&interrupt_handler_135, false, true);
    mEntries[136] = Entry((uintptr_t)&interrupt_handler_136, false, true);
    mEntries[137] = Entry((uintptr_t)&interrupt_handler_137, false, true);
    mEntries[138] = Entry((uintptr_t)&interrupt_handler_138, false, true);
    mEntries[139] = Entry((uintptr_t)&interrupt_handler_139, false, true);
    mEntries[140] = Entry((uintptr_t)&interrupt_handler_140, false, true);
    mEntries[141] = Entry((uintptr_t)&interrupt_handler_141, false, true);
    mEntries[142] = Entry((uintptr_t)&interrupt_handler_142, false, true);
    mEntries[143] = Entry((uintptr_t)&interrupt_handler_143, false, true);
    mEntries[144] = Entry((uintptr_t)&interrupt_handler_144, false, true);
    mEntries[145] = Entry((uintptr_t)&interrupt_handler_145, false, true);
    mEntries[146] = Entry((uintptr_t)&interrupt_handler_146, false, true);
    mEntries[147] = Entry((uintptr_t)&interrupt_handler_147, false, true);
    mEntries[148] = Entry((uintptr_t)&interrupt_handler_148, false, true);
    mEntries[149] = Entry((uintptr_t)&interrupt_handler_149, false, true);
    mEntries[150] = Entry((uintptr_t)&interrupt_handler_150, false, true);
    mEntries[151] = Entry((uintptr_t)&interrupt_handler_151, false, true);
    mEntries[152] = Entry((uintptr_t)&interrupt_handler_152, false, true);
    mEntries[153] = Entry((uintptr_t)&interrupt_handler_153, false, true);
    mEntries[154] = Entry((uintptr_t)&interrupt_handler_154, false, true);
    mEntries[155] = Entry((uintptr_t)&interrupt_handler_155, false, true);
    mEntries[156] = Entry((uintptr_t)&interrupt_handler_156, false, true);
    mEntries[157] = Entry((uintptr_t)&interrupt_handler_157, false, true);
    mEntries[158] = Entry((uintptr_t)&interrupt_handler_158, false, true);
    mEntries[159] = Entry((uintptr_t)&interrupt_handler_159, false, true);
    mEntries[160] = Entry((uintptr_t)&interrupt_handler_160, false, true);
    mEntries[161] = Entry((uintptr_t)&interrupt_handler_161, false, true);
    mEntries[162] = Entry((uintptr_t)&interrupt_handler_162, false, true);
    mEntries[163] = Entry((uintptr_t)&interrupt_handler_163, false, true);
    mEntries[164] = Entry((uintptr_t)&interrupt_handler_164, false, true);
    mEntries[165] = Entry((uintptr_t)&interrupt_handler_165, false, true);
    mEntries[166] = Entry((uintptr_t)&interrupt_handler_166, false, true);
    mEntries[167] = Entry((uintptr_t)&interrupt_handler_167, false, true);
    mEntries[168] = Entry((uintptr_t)&interrupt_handler_168, false, true);
    mEntries[169] = Entry((uintptr_t)&interrupt_handler_169, false, true);
    mEntries[170] = Entry((uintptr_t)&interrupt_handler_170, false, true);
    mEntries[171] = Entry((uintptr_t)&interrupt_handler_171, false, true);
    mEntries[172] = Entry((uintptr_t)&interrupt_handler_172, false, true);
    mEntries[173] = Entry((uintptr_t)&interrupt_handler_173, false, true);
    mEntries[174] = Entry((uintptr_t)&interrupt_handler_174, false, true);
    mEntries[175] = Entry((uintptr_t)&interrupt_handler_175, false, true);
    mEntries[176] = Entry((uintptr_t)&interrupt_handler_176, false, true);
    mEntries[177] = Entry((uintptr_t)&interrupt_handler_177, false, true);
    mEntries[178] = Entry((uintptr_t)&interrupt_handler_178, false, true);
    mEntries[179] = Entry((uintptr_t)&interrupt_handler_179, false, true);
    mEntries[180] = Entry((uintptr_t)&interrupt_handler_180, false, true);
    mEntries[181] = Entry((uintptr_t)&interrupt_handler_181, false, true);
    mEntries[182] = Entry((uintptr_t)&interrupt_handler_182, false, true);
    mEntries[183] = Entry((uintptr_t)&interrupt_handler_183, false, true);
    mEntries[184] = Entry((uintptr_t)&interrupt_handler_184, false, true);
    mEntries[185] = Entry((uintptr_t)&interrupt_handler_185, false, true);
    mEntries[186] = Entry((uintptr_t)&interrupt_handler_186, false, true);
    mEntries[187] = Entry((uintptr_t)&interrupt_handler_187, false, true);
    mEntries[188] = Entry((uintptr_t)&interrupt_handler_188, false, true);
    mEntries[189] = Entry((uintptr_t)&interrupt_handler_189, false, true);
    mEntries[190] = Entry((uintptr_t)&interrupt_handler_190, false, true);
    mEntries[191] = Entry((uintptr_t)&interrupt_handler_191, false, true);
    mEntries[192] = Entry((uintptr_t)&interrupt_handler_192, false, true);
    mEntries[193] = Entry((uintptr_t)&interrupt_handler_193, false, true);
    mEntries[194] = Entry((uintptr_t)&interrupt_handler_194, false, true);
    mEntries[195] = Entry((uintptr_t)&interrupt_handler_195, false, true);
    mEntries[196] = Entry((uintptr_t)&interrupt_handler_196, false, true);
    mEntries[197] = Entry((uintptr_t)&interrupt_handler_197, false, true);
    mEntries[198] = Entry((uintptr_t)&interrupt_handler_198, false, true);
    mEntries[199] = Entry((uintptr_t)&interrupt_handler_199, false, true);
    mEntries[200] = Entry((uintptr_t)&interrupt_handler_200, false, true);
    mEntries[201] = Entry((uintptr_t)&interrupt_handler_201, false, true);
    mEntries[202] = Entry((uintptr_t)&interrupt_handler_202, false, true);
    mEntries[203] = Entry((uintptr_t)&interrupt_handler_203, false, true);
    mEntries[204] = Entry((uintptr_t)&interrupt_handler_204, false, true);
    mEntries[205] = Entry((uintptr_t)&interrupt_handler_205, false, true);
    mEntries[206] = Entry((uintptr_t)&interrupt_handler_206, false, true);
    mEntries[207] = Entry((uintptr_t)&interrupt_handler_207, false, true);
    mEntries[208] = Entry((uintptr_t)&interrupt_handler_208, false, true);
    mEntries[209] = Entry((uintptr_t)&interrupt_handler_209, false, true);
    mEntries[210] = Entry((uintptr_t)&interrupt_handler_210, false, true);
    mEntries[211] = Entry((uintptr_t)&interrupt_handler_211, false, true);
    mEntries[212] = Entry((uintptr_t)&interrupt_handler_212, false, true);
    mEntries[213] = Entry((uintptr_t)&interrupt_handler_213, false, true);
    mEntries[214] = Entry((uintptr_t)&interrupt_handler_214, false, true);
    mEntries[215] = Entry((uintptr_t)&interrupt_handler_215, false, true);
    mEntries[216] = Entry((uintptr_t)&interrupt_handler_216, false, true);
    mEntries[217] = Entry((uintptr_t)&interrupt_handler_217, false, true);
    mEntries[218] = Entry((uintptr_t)&interrupt_handler_218, false, true);
    mEntries[219] = Entry((uintptr_t)&interrupt_handler_219, false, true);
    mEntries[220] = Entry((uintptr_t)&interrupt_handler_220, false, true);
    mEntries[221] = Entry((uintptr_t)&interrupt_handler_221, false, true);
    mEntries[222] = Entry((uintptr_t)&interrupt_handler_222, false, true);
    mEntries[223] = Entry((uintptr_t)&interrupt_handler_223, false, true);
    mEntries[224] = Entry((uintptr_t)&interrupt_handler_224, false, true);
    mEntries[225] = Entry((uintptr_t)&interrupt_handler_225, false, true);
    mEntries[226] = Entry((uintptr_t)&interrupt_handler_226, false, true);
    mEntries[227] = Entry((uintptr_t)&interrupt_handler_227, false, true);
    mEntries[228] = Entry((uintptr_t)&interrupt_handler_228, false, true);
    mEntries[229] = Entry((uintptr_t)&interrupt_handler_229, false, true);
    mEntries[230] = Entry((uintptr_t)&interrupt_handler_230, false, true);
    mEntries[231] = Entry((uintptr_t)&interrupt_handler_231, false, true);
    mEntries[232] = Entry((uintptr_t)&interrupt_handler_232, false, true);
    mEntries[233] = Entry((uintptr_t)&interrupt_handler_233, false, true);
    mEntries[234] = Entry((uintptr_t)&interrupt_handler_234, false, true);
    mEntries[235] = Entry((uintptr_t)&interrupt_handler_235, false, true);
    mEntries[236] = Entry((uintptr_t)&interrupt_handler_236, false, true);
    mEntries[237] = Entry((uintptr_t)&interrupt_handler_237, false, true);
    mEntries[238] = Entry((uintptr_t)&interrupt_handler_238, false, true);
    mEntries[239] = Entry((uintptr_t)&interrupt_handler_239, false, true);
    mEntries[240] = Entry((uintptr_t)&interrupt_handler_240, false, true);
    mEntries[241] = Entry((uintptr_t)&interrupt_handler_241, false, true);
    mEntries[242] = Entry((uintptr_t)&interrupt_handler_242, false, true);
    mEntries[243] = Entry((uintptr_t)&interrupt_handler_243, false, true);
    mEntries[244] = Entry((uintptr_t)&interrupt_handler_244, false, true);
    mEntries[245] = Entry((uintptr_t)&interrupt_handler_245, false, true);
    mEntries[246] = Entry((uintptr_t)&interrupt_handler_246, false, true);
    mEntries[247] = Entry((uintptr_t)&interrupt_handler_247, false, true);
    mEntries[248] = Entry((uintptr_t)&interrupt_handler_248, false, true);
    mEntries[249] = Entry((uintptr_t)&interrupt_handler_249, false, true);
    mEntries[250] = Entry((uintptr_t)&interrupt_handler_250, false, true);
    mEntries[251] = Entry((uintptr_t)&interrupt_handler_251, false, true);
    mEntries[252] = Entry((uintptr_t)&interrupt_handler_252, false, true);
    mEntries[253] = Entry((uintptr_t)&interrupt_handler_253, false, true);
    mEntries[254] = Entry((uintptr_t)&interrupt_handler_254, false, true);
    mEntries[255] = Entry((uintptr_t)&interrupt_handler_255, false, true);
}
