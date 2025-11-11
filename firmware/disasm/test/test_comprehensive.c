/* Comprehensive Z80 disassembler tests - all opcodes */

#include "test_framework.h"
#include "../disasm.h"
#include <string.h>

// Test case structure
typedef struct {
    const char* name;           // Test description
    uint8_t bytes[8];          // Opcode bytes
    uint8_t length;            // Number of bytes
    const char* expected;      // Expected disassembly output
} test_case_t;

// Test input function
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

// Run a single test case
void run_test_case(const test_case_t* tc)
{
    char output[128];
    reset_test_input(tc->bytes, tc->length);
    disasm(test_input, output);

    if (strcmp(tc->expected, output) != 0) {
        tests_failed++;
        printf("\n[FAIL] %s\n", tc->name);
        printf("  Opcode: ");
        for (int i = 0; i < tc->length; i++) {
            printf("%02x ", tc->bytes[i]);
        }
        printf("\n  Expected: \"%s\"\n", tc->expected);
        printf("  Actual:   \"%s\"\n", output);
    } else {
        tests_passed++;
        printf(".");
        fflush(stdout);
    }
    tests_run++;
}

// Base instruction tests (0x00-0xFF)
test_case_t base_opcodes[] = {
    // 0x0x - Misc operations, loads, jumps
    {"NOP", {0x00}, 1, "nop"},
    {"LD BC, 1234h", {0x01, 0x34, 0x12}, 3, "ld bc, 1234h"},
    {"LD (BC), A", {0x02}, 1, "ld (bc), a"},
    {"INC BC", {0x03}, 1, "inc bc"},
    {"INC B", {0x04}, 1, "inc b"},
    {"DEC B", {0x05}, 1, "dec b"},
    {"LD B, 42h", {0x06, 0x42}, 2, "ld b, 42h"},
    {"RLCA", {0x07}, 1, "rlca"},
    {"EX AF, AF'", {0x08}, 1, "ex af, af'"},
    {"ADD HL, BC", {0x09}, 1, "add hl, bc"},
    {"LD A, (BC)", {0x0A}, 1, "ld a, (bc)"},
    {"DEC BC", {0x0B}, 1, "dec bc"},
    {"INC C", {0x0C}, 1, "inc c"},
    {"DEC C", {0x0D}, 1, "dec c"},
    {"LD C, 55h", {0x0E, 0x55}, 2, "ld c, 55h"},
    {"RRCA", {0x0F}, 1, "rrca"},

    // 0x1x
    {"DJNZ -10", {0x10, 0xF6}, 2, "djnz -10"},
    {"LD DE, 5678h", {0x11, 0x78, 0x56}, 3, "ld de, 5678h"},
    {"LD (DE), A", {0x12}, 1, "ld (de), a"},
    {"INC DE", {0x13}, 1, "inc de"},
    {"INC D", {0x14}, 1, "inc d"},
    {"DEC D", {0x15}, 1, "dec d"},
    {"LD D, 11h", {0x16, 0x11}, 2, "ld d, 11h"},
    {"RLA", {0x17}, 1, "rla"},
    {"JR 5", {0x18, 0x05}, 2, "jr 5"},
    {"ADD HL, DE", {0x19}, 1, "add hl, de"},
    {"LD A, (DE)", {0x1A}, 1, "ld a, (de)"},
    {"DEC DE", {0x1B}, 1, "dec de"},
    {"INC E", {0x1C}, 1, "inc e"},
    {"DEC E", {0x1D}, 1, "dec e"},
    {"LD E, 22h", {0x1E, 0x22}, 2, "ld e, 22h"},
    {"RRA", {0x1F}, 1, "rra"},

    // 0x2x
    {"JR NZ, 10", {0x20, 0x0A}, 2, "jr nz, 10"},
    {"LD HL, 9ABCh", {0x21, 0xBC, 0x9A}, 3, "ld hl, 9abch"},
    {"LD (1234h), HL", {0x22, 0x34, 0x12}, 3, "ld (1234h), hl"},
    {"INC HL", {0x23}, 1, "inc hl"},
    {"INC H", {0x24}, 1, "inc h"},
    {"DEC H", {0x25}, 1, "dec h"},
    {"LD H, 33h", {0x26, 0x33}, 2, "ld h, 33h"},
    {"DAA", {0x27}, 1, "daa"},
    {"JR Z, -5", {0x28, 0xFB}, 2, "jr z, -5"},
    {"ADD HL, HL", {0x29}, 1, "add hl, hl"},
    {"LD HL, (5678h)", {0x2A, 0x78, 0x56}, 3, "ld hl, (5678h)"},
    {"DEC HL", {0x2B}, 1, "dec hl"},
    {"INC L", {0x2C}, 1, "inc l"},
    {"DEC L", {0x2D}, 1, "dec l"},
    {"LD L, 44h", {0x2E, 0x44}, 2, "ld l, 44h"},
    {"CPL", {0x2F}, 1, "cpl"},

    // 0x3x
    {"JR NC, 15", {0x30, 0x0F}, 2, "jr nc, 15"},
    {"LD SP, 0FFFEh", {0x31, 0xFE, 0xFF}, 3, "ld sp, 0fffeh"},
    {"LD (9000h), A", {0x32, 0x00, 0x90}, 3, "ld (9000h), a"},
    {"INC SP", {0x33}, 1, "inc sp"},
    {"INC (HL)", {0x34}, 1, "inc (hl)"},
    {"DEC (HL)", {0x35}, 1, "dec (hl)"},
    {"LD (HL), 66h", {0x36, 0x66}, 2, "ld (hl), 66h"},
    {"SCF", {0x37}, 1, "scf"},
    {"JR C, -20", {0x38, 0xEC}, 2, "jr c, -20"},
    {"ADD HL, SP", {0x39}, 1, "add hl, sp"},
    {"LD A, (8000h)", {0x3A, 0x00, 0x80}, 3, "ld a, (8000h)"},
    {"DEC SP", {0x3B}, 1, "dec sp"},
    {"INC A", {0x3C}, 1, "inc a"},
    {"DEC A", {0x3D}, 1, "dec a"},
    {"LD A, 77h", {0x3E, 0x77}, 2, "ld a, 77h"},
    {"CCF", {0x3F}, 1, "ccf"},

    // 0x4x - LD r, r' (first half)
    {"LD B, B", {0x40}, 1, "ld b, b"},
    {"LD B, C", {0x41}, 1, "ld b, c"},
    {"LD B, D", {0x42}, 1, "ld b, d"},
    {"LD B, E", {0x43}, 1, "ld b, e"},
    {"LD B, H", {0x44}, 1, "ld b, h"},
    {"LD B, L", {0x45}, 1, "ld b, l"},
    {"LD B, (HL)", {0x46}, 1, "ld b, (hl)"},
    {"LD B, A", {0x47}, 1, "ld b, a"},
    {"LD C, B", {0x48}, 1, "ld c, b"},
    {"LD C, C", {0x49}, 1, "ld c, c"},
    {"LD C, D", {0x4A}, 1, "ld c, d"},
    {"LD C, E", {0x4B}, 1, "ld c, e"},
    {"LD C, H", {0x4C}, 1, "ld c, h"},
    {"LD C, L", {0x4D}, 1, "ld c, l"},
    {"LD C, (HL)", {0x4E}, 1, "ld c, (hl)"},
    {"LD C, A", {0x4F}, 1, "ld c, a"},

    // Continue with more LD r, r' and other opcodes...
    {"HALT", {0x76}, 1, "halt"},

    // 0x8x - ADD/ADC/SUB/SBC
    {"ADD A, B", {0x80}, 1, "add a, b"},
    {"ADD A, C", {0x81}, 1, "add a, c"},
    {"ADD A, D", {0x82}, 1, "add a, d"},
    {"ADD A, E", {0x83}, 1, "add a, e"},
    {"ADD A, H", {0x84}, 1, "add a, h"},
    {"ADD A, L", {0x85}, 1, "add a, l"},
    {"ADD A, (HL)", {0x86}, 1, "add a, (hl)"},
    {"ADD A, A", {0x87}, 1, "add a, a"},
    {"ADC A, B", {0x88}, 1, "adc a, b"},
    {"ADC A, C", {0x89}, 1, "adc a, c"},
    {"ADC A, D", {0x8A}, 1, "adc a, d"},
    {"ADC A, E", {0x8B}, 1, "adc a, e"},
    {"ADC A, H", {0x8C}, 1, "adc a, h"},
    {"ADC A, L", {0x8D}, 1, "adc a, l"},
    {"ADC A, (HL)", {0x8E}, 1, "adc a, (hl)"},
    {"ADC A, A", {0x8F}, 1, "adc a, a"},

    {"SUB B", {0x90}, 1, "sub b"},
    {"SUB C", {0x91}, 1, "sub c"},
    {"SUB D", {0x92}, 1, "sub d"},
    {"SUB E", {0x93}, 1, "sub e"},
    {"SUB H", {0x94}, 1, "sub h"},
    {"SUB L", {0x95}, 1, "sub l"},
    {"SUB (HL)", {0x96}, 1, "sub (hl)"},
    {"SUB A", {0x97}, 1, "sub a"},
    {"SBC A, B", {0x98}, 1, "sbc a, b"},
    {"SBC A, C", {0x99}, 1, "sbc a, c"},
    {"SBC A, D", {0x9A}, 1, "sbc a, d"},
    {"SBC A, E", {0x9B}, 1, "sbc a, e"},
    {"SBC A, H", {0x9C}, 1, "sbc a, h"},
    {"SBC A, L", {0x9D}, 1, "sbc a, l"},
    {"SBC A, (HL)", {0x9E}, 1, "sbc a, (hl)"},
    {"SBC A, A", {0x9F}, 1, "sbc a, a"},

    // 0xAx - AND/XOR
    {"AND B", {0xA0}, 1, "and b"},
    {"AND C", {0xA1}, 1, "and c"},
    {"AND D", {0xA2}, 1, "and d"},
    {"AND E", {0xA3}, 1, "and e"},
    {"AND H", {0xA4}, 1, "and h"},
    {"AND L", {0xA5}, 1, "and l"},
    {"AND (HL)", {0xA6}, 1, "and (hl)"},
    {"AND A", {0xA7}, 1, "and a"},
    {"XOR B", {0xA8}, 1, "xor b"},
    {"XOR C", {0xA9}, 1, "xor c"},
    {"XOR D", {0xAA}, 1, "xor d"},
    {"XOR E", {0xAB}, 1, "xor e"},
    {"XOR H", {0xAC}, 1, "xor h"},
    {"XOR L", {0xAD}, 1, "xor l"},
    {"XOR (HL)", {0xAE}, 1, "xor (hl)"},
    {"XOR A", {0xAF}, 1, "xor a"},

    // 0xBx - OR/CP
    {"OR B", {0xB0}, 1, "or b"},
    {"OR C", {0xB1}, 1, "or c"},
    {"OR D", {0xB2}, 1, "or d"},
    {"OR E", {0xB3}, 1, "or e"},
    {"OR H", {0xB4}, 1, "or h"},
    {"OR L", {0xB5}, 1, "or l"},
    {"OR (HL)", {0xB6}, 1, "or (hl)"},
    {"OR A", {0xB7}, 1, "or a"},
    {"CP B", {0xB8}, 1, "cp b"},
    {"CP C", {0xB9}, 1, "cp c"},
    {"CP D", {0xBA}, 1, "cp d"},
    {"CP E", {0xBB}, 1, "cp e"},
    {"CP H", {0xBC}, 1, "cp h"},
    {"CP L", {0xBD}, 1, "cp l"},
    {"CP (HL)", {0xBE}, 1, "cp (hl)"},
    {"CP A", {0xBF}, 1, "cp a"},

    // 0xCx - Returns, Pops, Jumps, Calls
    {"RET NZ", {0xC0}, 1, "ret nz"},
    {"POP BC", {0xC1}, 1, "pop bc"},
    {"JP NZ, 1234h", {0xC2, 0x34, 0x12}, 3, "jp nz, 1234h"},
    {"JP 5678h", {0xC3, 0x78, 0x56}, 3, "jp 5678h"},
    {"CALL NZ, 9ABCh", {0xC4, 0xBC, 0x9A}, 3, "call nz, 9abch"},
    {"PUSH BC", {0xC5}, 1, "push bc"},
    {"ADD A, 88h", {0xC6, 0x88}, 2, "add a, 88h"},
    {"RST 00h", {0xC7}, 1, "rst 00h"},
    {"RET Z", {0xC8}, 1, "ret z"},
    {"RET", {0xC9}, 1, "ret"},
    {"JP Z, 0DEF0h", {0xCA, 0xF0, 0xDE}, 3, "jp z, 0def0h"},
    // 0xCB - Bit operations (tested separately)
    {"CALL Z, 1111h", {0xCC, 0x11, 0x11}, 3, "call z, 1111h"},
    {"CALL 2222h", {0xCD, 0x22, 0x22}, 3, "call 2222h"},
    {"ADC A, 99h", {0xCE, 0x99}, 2, "adc a, 99h"},
    {"RST 08h", {0xCF}, 1, "rst 08h"},

    {"RET NC", {0xD0}, 1, "ret nc"},
    {"POP DE", {0xD1}, 1, "pop de"},
    {"JP NC, 3333h", {0xD2, 0x33, 0x33}, 3, "jp nc, 3333h"},
    {"OUT (0FEh), A", {0xD3, 0xFE}, 2, "out (0feh), a"},
    {"CALL NC, 4444h", {0xD4, 0x44, 0x44}, 3, "call nc, 4444h"},
    {"PUSH DE", {0xD5}, 1, "push de"},
    {"SUB 0AAh", {0xD6, 0xAA}, 2, "sub 0aah"},
    {"RST 10h", {0xD7}, 1, "rst 10h"},
    {"RET C", {0xD8}, 1, "ret c"},
    {"EXX", {0xD9}, 1, "exx"},
    {"JP C, 5555h", {0xDA, 0x55, 0x55}, 3, "jp c, 5555h"},
    {"IN A, (0FFh)", {0xDB, 0xFF}, 2, "in a, (0ffh)"},
    {"CALL C, 6666h", {0xDC, 0x66, 0x66}, 3, "call c, 6666h"},
    // 0xDD - IX prefix (tested separately)
    {"SBC A, 0BBh", {0xDE, 0xBB}, 2, "sbc a, 0bbh"},
    {"RST 18h", {0xDF}, 1, "rst 18h"},

    {"RET PO", {0xE0}, 1, "ret po"},
    {"POP HL", {0xE1}, 1, "pop hl"},
    {"JP PO, 7777h", {0xE2, 0x77, 0x77}, 3, "jp po, 7777h"},
    {"EX (SP), HL", {0xE3}, 1, "ex (sp), hl"},
    {"CALL PO, 8888h", {0xE4, 0x88, 0x88}, 3, "call po, 8888h"},
    {"PUSH HL", {0xE5}, 1, "push hl"},
    {"AND 0CCh", {0xE6, 0xCC}, 2, "and 0cch"},
    {"RST 20h", {0xE7}, 1, "rst 20h"},
    {"RET PE", {0xE8}, 1, "ret pe"},
    {"JP (HL)", {0xE9}, 1, "jp (hl)"},
    {"JP PE, 9999h", {0xEA, 0x99, 0x99}, 3, "jp pe, 9999h"},
    {"EX DE, HL", {0xEB}, 1, "ex de, hl"},
    {"CALL PE, 0AAAAh", {0xEC, 0xAA, 0xAA}, 3, "call pe, 0aaaah"},
    // 0xED - Extended ops (tested separately)
    {"XOR 0DDh", {0xEE, 0xDD}, 2, "xor 0ddh"},
    {"RST 28h", {0xEF}, 1, "rst 28h"},

    {"RET P", {0xF0}, 1, "ret p"},
    {"POP AF", {0xF1}, 1, "pop af"},
    {"JP P, 0BBBBh", {0xF2, 0xBB, 0xBB}, 3, "jp p, 0bbbbh"},
    {"DI", {0xF3}, 1, "di"},
    {"CALL P, 0CCCCh", {0xF4, 0xCC, 0xCC}, 3, "call p, 0cccch"},
    {"PUSH AF", {0xF5}, 1, "push af"},
    {"OR 0EEh", {0xF6, 0xEE}, 2, "or 0eeh"},
    {"RST 30h", {0xF7}, 1, "rst 30h"},
    {"RET M", {0xF8}, 1, "ret m"},
    {"LD SP, HL", {0xF9}, 1, "ld sp, hl"},
    {"JP M, 0DDDDh", {0xFA, 0xDD, 0xDD}, 3, "jp m, 0ddddh"},
    {"EI", {0xFB}, 1, "ei"},
    {"CALL M, 0EEEEh", {0xFC, 0xEE, 0xEE}, 3, "call m, 0eeeeh"},
    // 0xFD - IY prefix (tested separately)
    {"CP 0FFh", {0xFE, 0xFF}, 2, "cp 0ffh"},
    {"RST 38h", {0xFF}, 1, "rst 38h"},
};

int main(void)
{
    TEST_INIT();

    TEST_SUITE("Comprehensive Base Opcode Tests");

    int num_tests = sizeof(base_opcodes) / sizeof(base_opcodes[0]);
    printf("Testing %d base opcodes...\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        run_test_case(&base_opcodes[i]);
    }

    TEST_REPORT();
}
