/* Unit tests for Z80 disassembler immediate() function */

#include "../disasm.h"
#include "../tokens.h"
#include "test_framework.h"
#include <string.h>

// Simple input function for testing
static uint8_t test_bytes[16];
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

void test_immediate_byte(void)
{
    TEST_CASE("test_immediate_byte");
    char output[32];
    uint8_t len;

    // Test BYTE operand - value starting with digit
    reset_test_input();
    test_bytes[0] = 0x42;
    memset(output, 0, sizeof(output));
    len = immediate(BYTE, -1, test_input, output);
    ASSERT_STR_EQ("42h", output, "BYTE 0x42 should produce '42h'");
    ASSERT(len > 0, "BYTE should return non-zero length");

    // Test BYTE operand - value starting with A-F (needs leading 0)
    reset_test_input();
    test_bytes[0] = 0xA5;
    memset(output, 0, sizeof(output));
    len = immediate(BYTE, -1, test_input, output);
    ASSERT_STR_EQ("0a5h", output, "BYTE 0xA5 should produce '0a5h'");
}

void test_immediate_word(void)
{
    TEST_CASE("test_immediate_word");
    char output[32];

    // Test WORD operand - little endian
    reset_test_input();
    test_bytes[0] = 0x34;
    test_bytes[1] = 0x12;
    memset(output, 0, sizeof(output));
    immediate(WORD, -1, test_input, output);
    ASSERT_STR_EQ("1234h", output, "WORD 0x1234 should produce '1234h'");

    // Test WORD operand - needs leading 0
    reset_test_input();
    test_bytes[0] = 0x00;
    test_bytes[1] = 0xA0;
    memset(output, 0, sizeof(output));
    immediate(WORD, -1, test_input, output);
    ASSERT_STR_EQ("0a000h", output, "WORD 0xA000 should produce '0a000h'");
}

void test_immediate_addr(void)
{
    TEST_CASE("test_immediate_addr");
    char output[32];

    // Test ADDR operand (indirect word)
    reset_test_input();
    test_bytes[0] = 0x00;
    test_bytes[1] = 0x80;
    memset(output, 0, sizeof(output));
    immediate(ADDR, -1, test_input, output);
    ASSERT_STR_EQ("(8000h)", output, "ADDR 0x8000 should produce '(8000h)'");
}

void test_immediate_port(void)
{
    TEST_CASE("test_immediate_port");
    char output[32];

    // Test PORT operand (indirect byte)
    reset_test_input();
    test_bytes[0] = 0xFF;
    memset(output, 0, sizeof(output));
    immediate(PORT, -1, test_input, output);
    ASSERT_STR_EQ("(0ffh)", output, "PORT 0xFF should produce '(0ffh)'");
}

void test_immediate_disp(void)
{
    TEST_CASE("test_immediate_disp");
    char output[32];

    // Test DISP operand - positive displacement
    reset_test_input();
    test_bytes[0] = 10;
    memset(output, 0, sizeof(output));
    immediate(DISP, -1, test_input, output);
    ASSERT_STR_EQ("10", output, "DISP 10 should produce '10'");

    // Test DISP operand - negative displacement
    reset_test_input();
    test_bytes[0] = (uint8_t)-5;
    memset(output, 0, sizeof(output));
    immediate(DISP, -1, test_input, output);
    ASSERT_STR_EQ("-5", output, "DISP -5 should produce '-5'");
}

void test_immediate_ixd_iyd(void)
{
    TEST_CASE("test_immediate_ixd_iyd");
    char output[32];

    // Test IXD with positive offset
    memset(output, 0, sizeof(output));
    immediate(IXD, 5, test_input, output);
    ASSERT_STR_EQ("(ix+5)", output, "IXD with offset 5 should produce '(ix+5)'");

    // Test IYD with positive offset
    memset(output, 0, sizeof(output));
    immediate(IYD, 10, test_input, output);
    ASSERT_STR_EQ("(iy+10)", output, "IYD with offset 10 should produce '(iy+10)'");

    // Test IXD with negative offset
    memset(output, 0, sizeof(output));
    immediate(IXD, -1, test_input, output);
    ASSERT_STR_EQ("(ix-1)", output, "IXD with offset -1 should produce '(ix-1)'");
}

int main(void)
{
    TEST_INIT();

    TEST_SUITE("Immediate Operand Tests");

    test_immediate_byte();
    test_immediate_word();
    test_immediate_addr();
    test_immediate_port();
    test_immediate_disp();
    test_immediate_ixd_iyd();

    TEST_REPORT();
}
