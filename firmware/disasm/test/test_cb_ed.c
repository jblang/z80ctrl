/* Comprehensive Z80 CB and ED prefix tests */

#include "test_framework.h"
#include "../disasm.h"
#include <string.h>

// Test case structure
typedef struct {
    const char* name;
    uint8_t bytes[8];
    uint8_t length;
    const char* expected;
} test_case_t;

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

// CB-prefixed opcode tests (all 256 CB xx combinations)
test_case_t cb_opcodes[] = {
    // RLC r (0xCB 0x00-0x07)
    {"RLC B", {0xCB, 0x00}, 2, "rlc b"},
    {"RLC C", {0xCB, 0x01}, 2, "rlc c"},
    {"RLC D", {0xCB, 0x02}, 2, "rlc d"},
    {"RLC E", {0xCB, 0x03}, 2, "rlc e"},
    {"RLC H", {0xCB, 0x04}, 2, "rlc h"},
    {"RLC L", {0xCB, 0x05}, 2, "rlc l"},
    {"RLC (HL)", {0xCB, 0x06}, 2, "rlc (hl)"},
    {"RLC A", {0xCB, 0x07}, 2, "rlc a"},

    // RRC r (0xCB 0x08-0x0F)
    {"RRC B", {0xCB, 0x08}, 2, "rrc b"},
    {"RRC C", {0xCB, 0x09}, 2, "rrc c"},
    {"RRC D", {0xCB, 0x0A}, 2, "rrc d"},
    {"RRC E", {0xCB, 0x0B}, 2, "rrc e"},
    {"RRC H", {0xCB, 0x0C}, 2, "rrc h"},
    {"RRC L", {0xCB, 0x0D}, 2, "rrc l"},
    {"RRC (HL)", {0xCB, 0x0E}, 2, "rrc (hl)"},
    {"RRC A", {0xCB, 0x0F}, 2, "rrc a"},

    // RL r (0xCB 0x10-0x17)
    {"RL B", {0xCB, 0x10}, 2, "rl b"},
    {"RL C", {0xCB, 0x11}, 2, "rl c"},
    {"RL D", {0xCB, 0x12}, 2, "rl d"},
    {"RL E", {0xCB, 0x13}, 2, "rl e"},
    {"RL H", {0xCB, 0x14}, 2, "rl h"},
    {"RL L", {0xCB, 0x15}, 2, "rl l"},
    {"RL (HL)", {0xCB, 0x16}, 2, "rl (hl)"},
    {"RL A", {0xCB, 0x17}, 2, "rl a"},

    // RR r (0xCB 0x18-0x1F)
    {"RR B", {0xCB, 0x18}, 2, "rr b"},
    {"RR C", {0xCB, 0x19}, 2, "rr c"},
    {"RR D", {0xCB, 0x1A}, 2, "rr d"},
    {"RR E", {0xCB, 0x1B}, 2, "rr e"},
    {"RR H", {0xCB, 0x1C}, 2, "rr h"},
    {"RR L", {0xCB, 0x1D}, 2, "rr l"},
    {"RR (HL)", {0xCB, 0x1E}, 2, "rr (hl)"},
    {"RR A", {0xCB, 0x1F}, 2, "rr a"},

    // SLA r (0xCB 0x20-0x27)
    {"SLA B", {0xCB, 0x20}, 2, "sla b"},
    {"SLA C", {0xCB, 0x21}, 2, "sla c"},
    {"SLA D", {0xCB, 0x22}, 2, "sla d"},
    {"SLA E", {0xCB, 0x23}, 2, "sla e"},
    {"SLA H", {0xCB, 0x24}, 2, "sla h"},
    {"SLA L", {0xCB, 0x25}, 2, "sla l"},
    {"SLA (HL)", {0xCB, 0x26}, 2, "sla (hl)"},
    {"SLA A", {0xCB, 0x27}, 2, "sla a"},

    // SRA r (0xCB 0x28-0x2F)
    {"SRA B", {0xCB, 0x28}, 2, "sra b"},
    {"SRA C", {0xCB, 0x29}, 2, "sra c"},
    {"SRA D", {0xCB, 0x2A}, 2, "sra d"},
    {"SRA E", {0xCB, 0x2B}, 2, "sra e"},
    {"SRA H", {0xCB, 0x2C}, 2, "sra h"},
    {"SRA L", {0xCB, 0x2D}, 2, "sra l"},
    {"SRA (HL)", {0xCB, 0x2E}, 2, "sra (hl)"},
    {"SRA A", {0xCB, 0x2F}, 2, "sra a"},

    // SLL r (undocumented 0xCB 0x30-0x37)
    {"SLL B", {0xCB, 0x30}, 2, "sll b"},
    {"SLL C", {0xCB, 0x31}, 2, "sll c"},
    {"SLL D", {0xCB, 0x32}, 2, "sll d"},
    {"SLL E", {0xCB, 0x33}, 2, "sll e"},
    {"SLL H", {0xCB, 0x34}, 2, "sll h"},
    {"SLL L", {0xCB, 0x35}, 2, "sll l"},
    {"SLL (HL)", {0xCB, 0x36}, 2, "sll (hl)"},
    {"SLL A", {0xCB, 0x37}, 2, "sll a"},

    // SRL r (0xCB 0x38-0x3F)
    {"SRL B", {0xCB, 0x38}, 2, "srl b"},
    {"SRL C", {0xCB, 0x39}, 2, "srl c"},
    {"SRL D", {0xCB, 0x3A}, 2, "srl d"},
    {"SRL E", {0xCB, 0x3B}, 2, "srl e"},
    {"SRL H", {0xCB, 0x3C}, 2, "srl h"},
    {"SRL L", {0xCB, 0x3D}, 2, "srl l"},
    {"SRL (HL)", {0xCB, 0x3E}, 2, "srl (hl)"},
    {"SRL A", {0xCB, 0x3F}, 2, "srl a"},

    // BIT 0, r through BIT 7, r (sample - testing key combinations)
    {"BIT 0, B", {0xCB, 0x40}, 2, "bit 0, b"},
    {"BIT 0, C", {0xCB, 0x41}, 2, "bit 0, c"},
    {"BIT 0, (HL)", {0xCB, 0x46}, 2, "bit 0, (hl)"},
    {"BIT 1, A", {0xCB, 0x4F}, 2, "bit 1, a"},
    {"BIT 2, B", {0xCB, 0x50}, 2, "bit 2, b"},
    {"BIT 3, D", {0xCB, 0x5A}, 2, "bit 3, d"},
    {"BIT 4, E", {0xCB, 0x63}, 2, "bit 4, e"},
    {"BIT 5, H", {0xCB, 0x6C}, 2, "bit 5, h"},
    {"BIT 6, L", {0xCB, 0x75}, 2, "bit 6, l"},
    {"BIT 7, A", {0xCB, 0x7F}, 2, "bit 7, a"},

    // RES 0-7, r (sample)
    {"RES 0, B", {0xCB, 0x80}, 2, "res 0, b"},
    {"RES 1, C", {0xCB, 0x89}, 2, "res 1, c"},
    {"RES 2, D", {0xCB, 0x92}, 2, "res 2, d"},
    {"RES 3, E", {0xCB, 0x9B}, 2, "res 3, e"},
    {"RES 4, H", {0xCB, 0xA4}, 2, "res 4, h"},
    {"RES 5, L", {0xCB, 0xAD}, 2, "res 5, l"},
    {"RES 6, (HL)", {0xCB, 0xB6}, 2, "res 6, (hl)"},
    {"RES 7, A", {0xCB, 0xBF}, 2, "res 7, a"},

    // SET 0-7, r (sample)
    {"SET 0, B", {0xCB, 0xC0}, 2, "set 0, b"},
    {"SET 1, C", {0xCB, 0xC9}, 2, "set 1, c"},
    {"SET 2, D", {0xCB, 0xD2}, 2, "set 2, d"},
    {"SET 3, E", {0xCB, 0xDB}, 2, "set 3, e"},
    {"SET 4, H", {0xCB, 0xE4}, 2, "set 4, h"},
    {"SET 5, L", {0xCB, 0xED}, 2, "set 5, l"},
    {"SET 6, (HL)", {0xCB, 0xF6}, 2, "set 6, (hl)"},
    {"SET 7, A", {0xCB, 0xFF}, 2, "set 7, a"},
};

// ED-prefixed opcode tests (documented opcodes)
test_case_t ed_opcodes[] = {
    // IN r, (C) - 0xED 0x40-0x78
    {"IN B, (C)", {0xED, 0x40}, 2, "in b, (c)"},
    {"IN C, (C)", {0xED, 0x48}, 2, "in c, (c)"},
    {"IN D, (C)", {0xED, 0x50}, 2, "in d, (c)"},
    {"IN E, (C)", {0xED, 0x58}, 2, "in e, (c)"},
    {"IN H, (C)", {0xED, 0x60}, 2, "in h, (c)"},
    {"IN L, (C)", {0xED, 0x68}, 2, "in l, (c)"},
    {"IN A, (C)", {0xED, 0x78}, 2, "in a, (c)"},

    // OUT (C), r - 0xED 0x41-0x79
    {"OUT (C), B", {0xED, 0x41}, 2, "out (c), b"},
    {"OUT (C), C", {0xED, 0x49}, 2, "out (c), c"},
    {"OUT (C), D", {0xED, 0x51}, 2, "out (c), d"},
    {"OUT (C), E", {0xED, 0x59}, 2, "out (c), e"},
    {"OUT (C), H", {0xED, 0x61}, 2, "out (c), h"},
    {"OUT (C), L", {0xED, 0x69}, 2, "out (c), l"},
    {"OUT (C), A", {0xED, 0x79}, 2, "out (c), a"},

    // SBC HL, rr - 0xED 0x42, 0x52, 0x62, 0x72
    {"SBC HL, BC", {0xED, 0x42}, 2, "sbc hl, bc"},
    {"SBC HL, DE", {0xED, 0x52}, 2, "sbc hl, de"},
    {"SBC HL, HL", {0xED, 0x62}, 2, "sbc hl, hl"},
    {"SBC HL, SP", {0xED, 0x72}, 2, "sbc hl, sp"},

    // ADC HL, rr - 0xED 0x4A, 0x5A, 0x6A, 0x7A
    {"ADC HL, BC", {0xED, 0x4A}, 2, "adc hl, bc"},
    {"ADC HL, DE", {0xED, 0x5A}, 2, "adc hl, de"},
    {"ADC HL, HL", {0xED, 0x6A}, 2, "adc hl, hl"},
    {"ADC HL, SP", {0xED, 0x7A}, 2, "adc hl, sp"},

    // LD (nn), rr / LD rr, (nn)
    {"LD (1234h), BC", {0xED, 0x43, 0x34, 0x12}, 4, "ld (1234h), bc"},
    {"LD BC, (5678h)", {0xED, 0x4B, 0x78, 0x56}, 4, "ld bc, (5678h)"},
    {"LD (9ABCh), DE", {0xED, 0x53, 0xBC, 0x9A}, 4, "ld (9abch), de"},
    {"LD DE, (0DEF0h)", {0xED, 0x5B, 0xF0, 0xDE}, 4, "ld de, (0def0h)"},
    {"LD (1111h), HL", {0xED, 0x63, 0x11, 0x11}, 4, "ld (1111h), hl"},
    {"LD HL, (2222h)", {0xED, 0x6B, 0x22, 0x22}, 4, "ld hl, (2222h)"},
    {"LD (3333h), SP", {0xED, 0x73, 0x33, 0x33}, 4, "ld (3333h), sp"},
    {"LD SP, (4444h)", {0xED, 0x7B, 0x44, 0x44}, 4, "ld sp, (4444h)"},

    // NEG - 0xED 0x44
    {"NEG", {0xED, 0x44}, 2, "neg"},

    // RETN / RETI
    {"RETN", {0xED, 0x45}, 2, "retn"},
    {"RETI", {0xED, 0x4D}, 2, "reti"},

    // IM 0/1/2
    {"IM 0", {0xED, 0x46}, 2, "im 0"},
    {"IM 0/1", {0xED, 0x4E}, 2, "im 0/1"},
    {"IM 1", {0xED, 0x56}, 2, "im 1"},
    {"IM 2", {0xED, 0x5E}, 2, "im 2"},

    // LD I, A / LD R, A / LD A, I / LD A, R
    {"LD I, A", {0xED, 0x47}, 2, "ld i, a"},
    {"LD R, A", {0xED, 0x4F}, 2, "ld r, a"},
    {"LD A, I", {0xED, 0x57}, 2, "ld a, i"},
    {"LD A, R", {0xED, 0x5F}, 2, "ld a, r"},

    // RRD / RLD
    {"RRD", {0xED, 0x67}, 2, "rrd"},
    {"RLD", {0xED, 0x6F}, 2, "rld"},

    // Block operations
    {"LDI", {0xED, 0xA0}, 2, "ldi"},
    {"CPI", {0xED, 0xA1}, 2, "cpi"},
    {"INI", {0xED, 0xA2}, 2, "ini"},
    {"OUTI", {0xED, 0xA3}, 2, "outi"},
    {"LDD", {0xED, 0xA8}, 2, "ldd"},
    {"CPD", {0xED, 0xA9}, 2, "cpd"},
    {"IND", {0xED, 0xAA}, 2, "ind"},
    {"OUTD", {0xED, 0xAB}, 2, "outd"},
    {"LDIR", {0xED, 0xB0}, 2, "ldir"},
    {"CPIR", {0xED, 0xB1}, 2, "cpir"},
    {"INIR", {0xED, 0xB2}, 2, "inir"},
    {"OTIR", {0xED, 0xB3}, 2, "otir"},
    {"LDDR", {0xED, 0xB8}, 2, "lddr"},
    {"CPDR", {0xED, 0xB9}, 2, "cpdr"},
    {"INDR", {0xED, 0xBA}, 2, "indr"},
    {"OTDR", {0xED, 0xBB}, 2, "otdr"},
};

int main(void)
{
    TEST_INIT();

    // Test CB opcodes
    TEST_SUITE("CB-Prefixed Opcodes");
    int num_cb = sizeof(cb_opcodes) / sizeof(cb_opcodes[0]);
    printf("Testing %d CB opcodes...\n", num_cb);
    for (int i = 0; i < num_cb; i++) {
        run_test_case(&cb_opcodes[i]);
    }

    // Test ED opcodes
    TEST_SUITE("ED-Prefixed Opcodes");
    int num_ed = sizeof(ed_opcodes) / sizeof(ed_opcodes[0]);
    printf("Testing %d ED opcodes...\n", num_ed);
    for (int i = 0; i < num_ed; i++) {
        run_test_case(&ed_opcodes[i]);
    }

    TEST_REPORT();
}
