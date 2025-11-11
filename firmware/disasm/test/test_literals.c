/* Unit tests for Z80 disassembler literal() function */

#include "test_framework.h"
#include "../disasm.h"
#include "../tokens.h"
#include <string.h>

void test_literal_single_char(void)
{
    TEST_CASE("test_literal_single_char");
    char output[16];
    uint8_t len;

    // Test single character literals
    memset(output, 0, sizeof(output));
    len = literal(B, output);
    ASSERT_EQ(1, len, "B register should be 1 char");
    ASSERT_STR_EQ("b", output, "B register should produce 'b'");

    memset(output, 0, sizeof(output));
    len = literal(A, output);
    ASSERT_EQ(1, len, "A register should be 1 char");
    ASSERT_STR_EQ("a", output, "A register should produce 'a'");

    memset(output, 0, sizeof(output));
    len = literal(N0, output);
    ASSERT_EQ(1, len, "N0 should be 1 char");
    ASSERT_STR_EQ("0", output, "N0 should produce '0'");
}

void test_literal_two_char(void)
{
    TEST_CASE("test_literal_two_char");
    char output[16];
    uint8_t len;

    // Test two character literals
    memset(output, 0, sizeof(output));
    len = literal(LD, output);
    ASSERT_EQ(2, len, "LD should be 2 chars");
    ASSERT_STR_EQ("ld", output, "LD should produce 'ld'");

    memset(output, 0, sizeof(output));
    len = literal(BC, output);
    ASSERT_EQ(2, len, "BC should be 2 chars");
    ASSERT_STR_EQ("bc", output, "BC should produce 'bc'");

    memset(output, 0, sizeof(output));
    len = literal(JP, output);
    ASSERT_EQ(2, len, "JP should be 2 chars");
    ASSERT_STR_EQ("jp", output, "JP should produce 'jp'");
}

void test_literal_three_char(void)
{
    TEST_CASE("test_literal_three_char");
    char output[16];
    uint8_t len;

    // Test three character literals
    memset(output, 0, sizeof(output));
    len = literal(ADD, output);
    ASSERT_EQ(3, len, "ADD should be 3 chars");
    ASSERT_STR_EQ("add", output, "ADD should produce 'add'");

    memset(output, 0, sizeof(output));
    len = literal(NOP, output);
    ASSERT_EQ(3, len, "NOP should be 3 chars");
    ASSERT_STR_EQ("nop", output, "NOP should produce 'nop'");

    memset(output, 0, sizeof(output));
    len = literal(RET, output);
    ASSERT_EQ(3, len, "RET should be 3 chars");
    ASSERT_STR_EQ("ret", output, "RET should produce 'ret'");
}

void test_literal_four_char(void)
{
    TEST_CASE("test_literal_four_char");
    char output[16];
    uint8_t len;

    // Test four character literals
    memset(output, 0, sizeof(output));
    len = literal(CALL, output);
    ASSERT_EQ(4, len, "CALL should be 4 chars");
    ASSERT_STR_EQ("call", output, "CALL should produce 'call'");

    memset(output, 0, sizeof(output));
    len = literal(PUSH, output);
    ASSERT_EQ(4, len, "PUSH should be 4 chars");
    ASSERT_STR_EQ("push", output, "PUSH should produce 'push'");

    memset(output, 0, sizeof(output));
    len = literal(HALT_, output);
    ASSERT_EQ(4, len, "HALT_ should be 4 chars");
    ASSERT_STR_EQ("halt", output, "HALT_ should produce 'halt'");
}

void test_literal_blank(void)
{
    TEST_CASE("test_literal_blank");
    char output[16];
    uint8_t len;

    // Test blank token
    memset(output, 'X', sizeof(output));
    len = literal(_, output);
    ASSERT_EQ(0, len, "Blank token should return 0 length");
}

int main(void)
{
    TEST_INIT();

    TEST_SUITE("Literal Token Tests");

    test_literal_single_char();
    test_literal_two_char();
    test_literal_three_char();
    test_literal_four_char();
    test_literal_blank();

    TEST_REPORT();
}
