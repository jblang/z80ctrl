/* Integration tests for complete Z80 disassembler */

#include "test_framework.h"
#include "../disasm.h"
#include "../tokens.h"
#include <string.h>

// Test instruction bytes
static uint8_t test_bytes[32];
static int test_byte_index = 0;

uint8_t test_input(void)
{
    return test_bytes[test_byte_index++];
}

void reset_test_input(void)
{
    test_byte_index = 0;
    memset(test_bytes, 0, sizeof(test_bytes));
}

void test_disasm_nop(void)
{
    TEST_CASE("test_disasm_nop");
    char output[64];

    reset_test_input();
    test_bytes[0] = 0x00;  // NOP
    disasm(test_input, output);
    ASSERT_STR_EQ("nop", output, "NOP should disassemble correctly");
}

void test_disasm_ld_imm(void)
{
    TEST_CASE("test_disasm_ld_imm");
    char output[64];

    // LD A, 42h
    reset_test_input();
    test_bytes[0] = 0x3E;
    test_bytes[1] = 0x42;
    disasm(test_input, output);
    ASSERT_STR_EQ("ld a, 42h", output, "LD A, 42h should disassemble correctly");

    // LD B, 0FFh
    reset_test_input();
    test_bytes[0] = 0x06;
    test_bytes[1] = 0xFF;
    disasm(test_input, output);
    ASSERT_STR_EQ("ld b, 0ffh", output, "LD B, 0FFh should disassemble correctly");
}

void test_disasm_ld_reg_reg(void)
{
    TEST_CASE("test_disasm_ld_reg_reg");
    char output[64];

    // LD B, C
    reset_test_input();
    test_bytes[0] = 0x41;
    disasm(test_input, output);
    ASSERT_STR_EQ("ld b, c", output, "LD B, C should disassemble correctly");

    // LD A, A
    reset_test_input();
    test_bytes[0] = 0x7F;
    disasm(test_input, output);
    ASSERT_STR_EQ("ld a, a", output, "LD A, A should disassemble correctly");

    // LD H, L
    reset_test_input();
    test_bytes[0] = 0x65;
    disasm(test_input, output);
    ASSERT_STR_EQ("ld h, l", output, "LD H, L should disassemble correctly");
}

void test_disasm_ld_16bit(void)
{
    TEST_CASE("test_disasm_ld_16bit");
    char output[64];

    // LD BC, 1234h
    reset_test_input();
    test_bytes[0] = 0x01;
    test_bytes[1] = 0x34;
    test_bytes[2] = 0x12;
    disasm(test_input, output);
    ASSERT_STR_EQ("ld bc, 1234h", output, "LD BC, 1234h should disassemble correctly");

    // LD HL, 0ABCDh
    reset_test_input();
    test_bytes[0] = 0x21;
    test_bytes[1] = 0xCD;
    test_bytes[2] = 0xAB;
    disasm(test_input, output);
    ASSERT_STR_EQ("ld hl, 0abcdh", output, "LD HL, 0ABCDh should disassemble correctly");
}

void test_disasm_alu_ops(void)
{
    TEST_CASE("test_disasm_alu_ops");
    char output[64];

    // ADD A, B
    reset_test_input();
    test_bytes[0] = 0x80;
    disasm(test_input, output);
    ASSERT_STR_EQ("add a, b", output, "ADD A, B should disassemble correctly");

    // SUB C
    reset_test_input();
    test_bytes[0] = 0x91;
    disasm(test_input, output);
    ASSERT_STR_EQ("sub c", output, "SUB C should disassemble correctly");

    // AND 55h
    reset_test_input();
    test_bytes[0] = 0xE6;
    test_bytes[1] = 0x55;
    disasm(test_input, output);
    ASSERT_STR_EQ("and 55h", output, "AND 55h should disassemble correctly");

    // CP 0AAh
    reset_test_input();
    test_bytes[0] = 0xFE;
    test_bytes[1] = 0xAA;
    disasm(test_input, output);
    ASSERT_STR_EQ("cp 0aah", output, "CP 0AAh should disassemble correctly");
}

void test_disasm_jump_call(void)
{
    TEST_CASE("test_disasm_jump_call");
    char output[64];

    // JP 1234h
    reset_test_input();
    test_bytes[0] = 0xC3;
    test_bytes[1] = 0x34;
    test_bytes[2] = 0x12;
    disasm(test_input, output);
    ASSERT_STR_EQ("jp 1234h", output, "JP 1234h should disassemble correctly");

    // CALL NZ, 5678h
    reset_test_input();
    test_bytes[0] = 0xC4;
    test_bytes[1] = 0x78;
    test_bytes[2] = 0x56;
    disasm(test_input, output);
    ASSERT_STR_EQ("call nz, 5678h", output, "CALL NZ, 5678h should disassemble correctly");

    // RET
    reset_test_input();
    test_bytes[0] = 0xC9;
    disasm(test_input, output);
    ASSERT_STR_EQ("ret", output, "RET should disassemble correctly");
}

void test_disasm_jr(void)
{
    TEST_CASE("test_disasm_jr");
    char output[64];

    // JR 10
    reset_test_input();
    test_bytes[0] = 0x18;
    test_bytes[1] = 10;
    disasm(test_input, output);
    ASSERT_STR_EQ("jr 10", output, "JR 10 should disassemble correctly");

    // JR Z, -5
    reset_test_input();
    test_bytes[0] = 0x28;
    test_bytes[1] = (uint8_t)-5;
    disasm(test_input, output);
    ASSERT_STR_EQ("jr z, -5", output, "JR Z, -5 should disassemble correctly");
}

void test_disasm_cb_rotate(void)
{
    TEST_CASE("test_disasm_cb_rotate");
    char output[64];

    // RLC B
    reset_test_input();
    test_bytes[0] = 0xCB;
    test_bytes[1] = 0x00;
    disasm(test_input, output);
    ASSERT_STR_EQ("rlc b", output, "RLC B should disassemble correctly");

    // RR A
    reset_test_input();
    test_bytes[0] = 0xCB;
    test_bytes[1] = 0x1F;
    disasm(test_input, output);
    ASSERT_STR_EQ("rr a", output, "RR A should disassemble correctly");

    // SLA H
    reset_test_input();
    test_bytes[0] = 0xCB;
    test_bytes[1] = 0x24;
    disasm(test_input, output);
    ASSERT_STR_EQ("sla h", output, "SLA H should disassemble correctly");
}

void test_disasm_cb_bit(void)
{
    TEST_CASE("test_disasm_cb_bit");
    char output[64];

    // BIT 0, B
    reset_test_input();
    test_bytes[0] = 0xCB;
    test_bytes[1] = 0x40;
    disasm(test_input, output);
    ASSERT_STR_EQ("bit 0, b", output, "BIT 0, B should disassemble correctly");

    // BIT 7, A
    reset_test_input();
    test_bytes[0] = 0xCB;
    test_bytes[1] = 0x7F;
    disasm(test_input, output);
    ASSERT_STR_EQ("bit 7, a", output, "BIT 7, A should disassemble correctly");

    // RES 3, D
    reset_test_input();
    test_bytes[0] = 0xCB;
    test_bytes[1] = 0x9A;
    disasm(test_input, output);
    ASSERT_STR_EQ("res 3, d", output, "RES 3, D should disassemble correctly");

    // SET 5, E
    reset_test_input();
    test_bytes[0] = 0xCB;
    test_bytes[1] = 0xEB;
    disasm(test_input, output);
    ASSERT_STR_EQ("set 5, e", output, "SET 5, E should disassemble correctly");
}

void test_disasm_halt(void)
{
    TEST_CASE("test_disasm_halt");
    char output[64];

    reset_test_input();
    test_bytes[0] = 0x76;
    disasm(test_input, output);
    ASSERT_STR_EQ("halt", output, "HALT should disassemble correctly");
}

void test_disasm_push_pop(void)
{
    TEST_CASE("test_disasm_push_pop");
    char output[64];

    // PUSH BC
    reset_test_input();
    test_bytes[0] = 0xC5;
    disasm(test_input, output);
    ASSERT_STR_EQ("push bc", output, "PUSH BC should disassemble correctly");

    // POP HL
    reset_test_input();
    test_bytes[0] = 0xE1;
    disasm(test_input, output);
    ASSERT_STR_EQ("pop hl", output, "POP HL should disassemble correctly");
}

void test_disasm_inc_dec(void)
{
    TEST_CASE("test_disasm_inc_dec");
    char output[64];

    // INC B
    reset_test_input();
    test_bytes[0] = 0x04;
    disasm(test_input, output);
    ASSERT_STR_EQ("inc b", output, "INC B should disassemble correctly");

    // DEC C
    reset_test_input();
    test_bytes[0] = 0x0D;
    disasm(test_input, output);
    ASSERT_STR_EQ("dec c", output, "DEC C should disassemble correctly");

    // INC BC
    reset_test_input();
    test_bytes[0] = 0x03;
    disasm(test_input, output);
    ASSERT_STR_EQ("inc bc", output, "INC BC should disassemble correctly");

    // DEC HL
    reset_test_input();
    test_bytes[0] = 0x2B;
    disasm(test_input, output);
    ASSERT_STR_EQ("dec hl", output, "DEC HL should disassemble correctly");
}

int main(void)
{
    TEST_INIT();

    TEST_SUITE("Disassembler Integration Tests");

    test_disasm_nop();
    test_disasm_ld_imm();
    test_disasm_ld_reg_reg();
    test_disasm_ld_16bit();
    test_disasm_alu_ops();
    test_disasm_jump_call();
    test_disasm_jr();
    test_disasm_cb_rotate();
    test_disasm_cb_bit();
    test_disasm_halt();
    test_disasm_push_pop();
    test_disasm_inc_dec();

    TEST_REPORT();
}
