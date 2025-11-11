/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2025 J.B. Langston
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file test_ix_iy.c - Comprehensive tests for IX/IY indexed instructions
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "test_framework.h"
#include "../disasm.h"

// Test input management
static uint8_t test_bytes[32];
static int test_byte_index = 0;

uint8_t test_input(void)
{
    return test_bytes[test_byte_index++];
}

void reset_test_input(const uint8_t* bytes, uint8_t length)
{
    test_byte_index = 0;
    memset(test_bytes, 0, sizeof(test_bytes));
    memcpy(test_bytes, bytes, length);
}

// Test data structure
typedef struct {
    const char* name;
    uint8_t bytes[6];  // max instruction length
    uint8_t len;
    const char* expected;
} test_case_t;

// DD-prefixed IX instructions
test_case_t ix_tests[] = {
    // Basic IX instructions
    {"LD IX, 1234h", {0xDD, 0x21, 0x34, 0x12}, 4, "ld ix, 1234h"},
    {"ADD IX, BC", {0xDD, 0x09}, 2, "add ix, bc"},
    {"ADD IX, DE", {0xDD, 0x19}, 2, "add ix, de"},
    {"ADD IX, IX", {0xDD, 0x29}, 2, "add ix, ix"},
    {"ADD IX, SP", {0xDD, 0x39}, 2, "add ix, sp"},
    {"LD (5678h), IX", {0xDD, 0x22, 0x78, 0x56}, 4, "ld (5678h), ix"},
    {"LD IX, (9ABCh)", {0xDD, 0x2A, 0xBC, 0x9A}, 4, "ld ix, (9abch)"},
    {"INC IX", {0xDD, 0x23}, 2, "inc ix"},
    {"DEC IX", {0xDD, 0x2B}, 2, "dec ix"},

    // IX high/low register ops
    {"INC IXH", {0xDD, 0x24}, 2, "inc ixh"},
    {"DEC IXH", {0xDD, 0x25}, 2, "dec ixh"},
    {"LD IXH, 55h", {0xDD, 0x26, 0x55}, 3, "ld ixh, 55h"},
    {"INC IXL", {0xDD, 0x2C}, 2, "inc ixl"},
    {"DEC IXL", {0xDD, 0x2D}, 2, "dec ixl"},
    {"LD IXL, 0AAh", {0xDD, 0x2E, 0xAA}, 3, "ld ixl, 0aah"},

    // Indexed loads
    {"INC (IX+5)", {0xDD, 0x34, 0x05}, 3, "inc (ix+5)"},
    {"DEC (IX+10)", {0xDD, 0x35, 0x0A}, 3, "dec (ix+10)"},
    {"LD (IX+0), 99h", {0xDD, 0x36, 0x00, 0x99}, 4, "ld (ix+0), 99h"},
    {"LD (IX-1), 0FFh", {0xDD, 0x36, 0xFF, 0xFF}, 4, "ld (ix-1), 0ffh"},

    // LD r, (IX+d)
    {"LD B, (IX+1)", {0xDD, 0x46, 0x01}, 3, "ld b, (ix+1)"},
    {"LD C, (IX+2)", {0xDD, 0x4E, 0x02}, 3, "ld c, (ix+2)"},
    {"LD D, (IX+3)", {0xDD, 0x56, 0x03}, 3, "ld d, (ix+3)"},
    {"LD E, (IX+4)", {0xDD, 0x5E, 0x04}, 3, "ld e, (ix+4)"},
    {"LD H, (IX+5)", {0xDD, 0x66, 0x05}, 3, "ld h, (ix+5)"},
    {"LD L, (IX+6)", {0xDD, 0x6E, 0x06}, 3, "ld l, (ix+6)"},
    {"LD A, (IX+7)", {0xDD, 0x7E, 0x07}, 3, "ld a, (ix+7)"},

    // LD (IX+d), r
    {"LD (IX+8), B", {0xDD, 0x70, 0x08}, 3, "ld (ix+8), b"},
    {"LD (IX+9), C", {0xDD, 0x71, 0x09}, 3, "ld (ix+9), c"},
    {"LD (IX+10), D", {0xDD, 0x72, 0x0A}, 3, "ld (ix+10), d"},
    {"LD (IX+11), E", {0xDD, 0x73, 0x0B}, 3, "ld (ix+11), e"},
    {"LD (IX+12), H", {0xDD, 0x74, 0x0C}, 3, "ld (ix+12), h"},
    {"LD (IX+13), L", {0xDD, 0x75, 0x0D}, 3, "ld (ix+13), l"},
    {"LD (IX+14), A", {0xDD, 0x77, 0x0E}, 3, "ld (ix+14), a"},

    // ALU ops with (IX+d)
    {"ADD A, (IX+15)", {0xDD, 0x86, 0x0F}, 3, "add a, (ix+15)"},
    {"ADC A, (IX+16)", {0xDD, 0x8E, 0x10}, 3, "adc a, (ix+16)"},
    {"SUB (IX+17)", {0xDD, 0x96, 0x11}, 3, "sub (ix+17)"},
    {"SBC A, (IX+18)", {0xDD, 0x9E, 0x12}, 3, "sbc a, (ix+18)"},
    {"AND (IX+19)", {0xDD, 0xA6, 0x13}, 3, "and (ix+19)"},
    {"XOR (IX+20)", {0xDD, 0xAE, 0x14}, 3, "xor (ix+20)"},
    {"OR (IX+21)", {0xDD, 0xB6, 0x15}, 3, "or (ix+21)"},
    {"CP (IX+22)", {0xDD, 0xBE, 0x16}, 3, "cp (ix+22)"},

    // Stack ops
    {"POP IX", {0xDD, 0xE1}, 2, "pop ix"},
    {"EX (SP), IX", {0xDD, 0xE3}, 2, "ex (sp), ix"},
    {"PUSH IX", {0xDD, 0xE5}, 2, "push ix"},
    {"JP (IX)", {0xDD, 0xE9}, 2, "jp (ix)"},
    {"LD SP, IX", {0xDD, 0xF9}, 2, "ld sp, ix"},
};

// FD-prefixed IY instructions
test_case_t iy_tests[] = {
    // Basic IY instructions
    {"LD IY, 0FEDCh", {0xFD, 0x21, 0xDC, 0xFE}, 4, "ld iy, 0fedch"},
    {"ADD IY, BC", {0xFD, 0x09}, 2, "add iy, bc"},
    {"ADD IY, DE", {0xFD, 0x19}, 2, "add iy, de"},
    {"ADD IY, IY", {0xFD, 0x29}, 2, "add iy, iy"},
    {"ADD IY, SP", {0xFD, 0x39}, 2, "add iy, sp"},
    {"LD (1111h), IY", {0xFD, 0x22, 0x11, 0x11}, 4, "ld (1111h), iy"},
    {"LD IY, (2222h)", {0xFD, 0x2A, 0x22, 0x22}, 4, "ld iy, (2222h)"},
    {"INC IY", {0xFD, 0x23}, 2, "inc iy"},
    {"DEC IY", {0xFD, 0x2B}, 2, "dec iy"},

    // IY high/low register ops
    {"INC IYH", {0xFD, 0x24}, 2, "inc iyh"},
    {"DEC IYH", {0xFD, 0x25}, 2, "dec iyh"},
    {"LD IYH, 0CCh", {0xFD, 0x26, 0xCC}, 3, "ld iyh, 0cch"},
    {"INC IYL", {0xFD, 0x2C}, 2, "inc iyl"},
    {"DEC IYL", {0xFD, 0x2D}, 2, "dec iyl"},
    {"LD IYL, 33h", {0xFD, 0x2E, 0x33}, 3, "ld iyl, 33h"},

    // Indexed loads
    {"INC (IY-5)", {0xFD, 0x34, 0xFB}, 3, "inc (iy-5)"},
    {"DEC (IY-10)", {0xFD, 0x35, 0xF6}, 3, "dec (iy-10)"},
    {"LD (IY+127), 7Fh", {0xFD, 0x36, 0x7F, 0x7F}, 4, "ld (iy+127), 7fh"},

    // LD r, (IY+d)
    {"LD B, (IY+30)", {0xFD, 0x46, 0x1E}, 3, "ld b, (iy+30)"},
    {"LD C, (IY+31)", {0xFD, 0x4E, 0x1F}, 3, "ld c, (iy+31)"},
    {"LD A, (IY+40)", {0xFD, 0x7E, 0x28}, 3, "ld a, (iy+40)"},

    // LD (IY+d), r
    {"LD (IY+50), B", {0xFD, 0x70, 0x32}, 3, "ld (iy+50), b"},
    {"LD (IY+60), A", {0xFD, 0x77, 0x3C}, 3, "ld (iy+60), a"},

    // ALU ops with (IY+d)
    {"ADD A, (IY-15)", {0xFD, 0x86, 0xF1}, 3, "add a, (iy-15)"},
    {"SUB (IY-20)", {0xFD, 0x96, 0xEC}, 3, "sub (iy-20)"},
    {"AND (IY-25)", {0xFD, 0xA6, 0xE7}, 3, "and (iy-25)"},
    {"CP (IY-30)", {0xFD, 0xBE, 0xE2}, 3, "cp (iy-30)"},

    // Stack ops
    {"POP IY", {0xFD, 0xE1}, 2, "pop iy"},
    {"EX (SP), IY", {0xFD, 0xE3}, 2, "ex (sp), iy"},
    {"PUSH IY", {0xFD, 0xE5}, 2, "push iy"},
    {"JP (IY)", {0xFD, 0xE9}, 2, "jp (iy)"},
    {"LD SP, IY", {0xFD, 0xF9}, 2, "ld sp, iy"},
};

// DDCB-prefixed IX bit instructions
test_case_t ixcb_tests[] = {
    // Rotate/shift (IX+d)
    {"RLC (IX+1)", {0xDD, 0xCB, 0x01, 0x06}, 4, "rlc (ix+1)"},
    {"RRC (IX+2)", {0xDD, 0xCB, 0x02, 0x0E}, 4, "rrc (ix+2)"},
    {"RL (IX+3)", {0xDD, 0xCB, 0x03, 0x16}, 4, "rl (ix+3)"},
    {"RR (IX+4)", {0xDD, 0xCB, 0x04, 0x1E}, 4, "rr (ix+4)"},
    {"SLA (IX+5)", {0xDD, 0xCB, 0x05, 0x26}, 4, "sla (ix+5)"},
    {"SRA (IX+6)", {0xDD, 0xCB, 0x06, 0x2E}, 4, "sra (ix+6)"},
    {"SLL (IX+7)", {0xDD, 0xCB, 0x07, 0x36}, 4, "sll (ix+7)"},
    {"SRL (IX+8)", {0xDD, 0xCB, 0x08, 0x3E}, 4, "srl (ix+8)"},

    // BIT n, (IX+d)
    {"BIT 0, (IX+10)", {0xDD, 0xCB, 0x0A, 0x46}, 4, "bit 0, (ix+10)"},
    {"BIT 1, (IX+11)", {0xDD, 0xCB, 0x0B, 0x4E}, 4, "bit 1, (ix+11)"},
    {"BIT 2, (IX+12)", {0xDD, 0xCB, 0x0C, 0x56}, 4, "bit 2, (ix+12)"},
    {"BIT 3, (IX+13)", {0xDD, 0xCB, 0x0D, 0x5E}, 4, "bit 3, (ix+13)"},
    {"BIT 4, (IX+14)", {0xDD, 0xCB, 0x0E, 0x66}, 4, "bit 4, (ix+14)"},
    {"BIT 5, (IX+15)", {0xDD, 0xCB, 0x0F, 0x6E}, 4, "bit 5, (ix+15)"},
    {"BIT 6, (IX+16)", {0xDD, 0xCB, 0x10, 0x76}, 4, "bit 6, (ix+16)"},
    {"BIT 7, (IX+17)", {0xDD, 0xCB, 0x11, 0x7E}, 4, "bit 7, (ix+17)"},

    // RES n, (IX+d)
    {"RES 0, (IX+20)", {0xDD, 0xCB, 0x14, 0x86}, 4, "res 0, (ix+20)"},
    {"RES 1, (IX+21)", {0xDD, 0xCB, 0x15, 0x8E}, 4, "res 1, (ix+21)"},
    {"RES 7, (IX+27)", {0xDD, 0xCB, 0x1B, 0xBE}, 4, "res 7, (ix+27)"},

    // SET n, (IX+d)
    {"SET 0, (IX+30)", {0xDD, 0xCB, 0x1E, 0xC6}, 4, "set 0, (ix+30)"},
    {"SET 7, (IX+37)", {0xDD, 0xCB, 0x25, 0xFE}, 4, "set 7, (ix+37)"},
};

// FDCB-prefixed IY bit instructions
test_case_t iycb_tests[] = {
    // Rotate/shift (IY+d)
    {"RLC (IY-1)", {0xFD, 0xCB, 0xFF, 0x06}, 4, "rlc (iy-1)"},
    {"RRC (IY-2)", {0xFD, 0xCB, 0xFE, 0x0E}, 4, "rrc (iy-2)"},
    {"RL (IY-3)", {0xFD, 0xCB, 0xFD, 0x16}, 4, "rl (iy-3)"},
    {"RR (IY-4)", {0xFD, 0xCB, 0xFC, 0x1E}, 4, "rr (iy-4)"},
    {"SLA (IY-5)", {0xFD, 0xCB, 0xFB, 0x26}, 4, "sla (iy-5)"},
    {"SRA (IY-6)", {0xFD, 0xCB, 0xFA, 0x2E}, 4, "sra (iy-6)"},
    {"SRL (IY-8)", {0xFD, 0xCB, 0xF8, 0x3E}, 4, "srl (iy-8)"},

    // BIT n, (IY+d)
    {"BIT 0, (IY+50)", {0xFD, 0xCB, 0x32, 0x46}, 4, "bit 0, (iy+50)"},
    {"BIT 3, (IY+60)", {0xFD, 0xCB, 0x3C, 0x5E}, 4, "bit 3, (iy+60)"},
    {"BIT 7, (IY+70)", {0xFD, 0xCB, 0x46, 0x7E}, 4, "bit 7, (iy+70)"},

    // RES n, (IY+d)
    {"RES 0, (IY+80)", {0xFD, 0xCB, 0x50, 0x86}, 4, "res 0, (iy+80)"},
    {"RES 7, (IY+90)", {0xFD, 0xCB, 0x5A, 0xBE}, 4, "res 7, (iy+90)"},

    // SET n, (IY+d)
    {"SET 0, (IY+100)", {0xFD, 0xCB, 0x64, 0xC6}, 4, "set 0, (iy+100)"},
    {"SET 7, (IY+110)", {0xFD, 0xCB, 0x6E, 0xFE}, 4, "set 7, (iy+110)"},
};

// Test runner
void run_test_set(const char* suite_name, test_case_t* tests, int count) {
    TEST_SUITE(suite_name);
    printf("Testing %d opcodes...\n", count);

    for (int i = 0; i < count; i++) {
        test_case_t* t = &tests[i];

        // Reset input and disassemble
        reset_test_input(t->bytes, t->len);
        char output[128];
        disasm(test_input, output);

        // Check result
        if (strcmp(output, t->expected) != 0) {
            tests_failed++;
            printf("\n[FAIL] %s\n", t->name);
            printf("  Opcode: ");
            for (uint8_t j = 0; j < t->len; j++) {
                printf("%02x ", t->bytes[j]);
            }
            printf("\n");
            printf("  Expected: \"%s\"\n", t->expected);
            printf("  Actual:   \"%s\"\n", output);
        } else {
            tests_passed++;
            printf(".");
            fflush(stdout);
        }
        tests_run++;
    }
    printf("\n");
}

int main() {
    TEST_INIT();

    run_test_set("DD-Prefixed IX Instructions", ix_tests,
                 sizeof(ix_tests) / sizeof(ix_tests[0]));
    run_test_set("FD-Prefixed IY Instructions", iy_tests,
                 sizeof(iy_tests) / sizeof(iy_tests[0]));
    run_test_set("DDCB-Prefixed IX Bit Instructions", ixcb_tests,
                 sizeof(ixcb_tests) / sizeof(ixcb_tests[0]));
    run_test_set("FDCB-Prefixed IY Bit Instructions", iycb_tests,
                 sizeof(iycb_tests) / sizeof(iycb_tests[0]));

    TEST_REPORT();
}
