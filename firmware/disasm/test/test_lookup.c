/* Unit tests for Z80 disassembler lookup functions */

#include "test_framework.h"
#include "../tokens.h"
#include <stdint.h>
#include <string.h>

// Forward declarations from lookup.c
extern uint8_t t_lookup(uint8_t tok, uint8_t y, uint8_t z);
extern instr_t decode(uint8_t prefix, uint8_t opcode);

void test_t_lookup_y_reg8(void)
{
    TEST_CASE("test_t_lookup_y_reg8");

    // Y_REG8 token should look up 8-bit registers by y field
    ASSERT_EQ(B, t_lookup(Y_REG8, 0, 0), "y=0 should give B");
    ASSERT_EQ(C, t_lookup(Y_REG8, 1, 0), "y=1 should give C");
    ASSERT_EQ(D, t_lookup(Y_REG8, 2, 0), "y=2 should give D");
    ASSERT_EQ(E, t_lookup(Y_REG8, 3, 0), "y=3 should give E");
    ASSERT_EQ(H, t_lookup(Y_REG8, 4, 0), "y=4 should give H");
    ASSERT_EQ(L, t_lookup(Y_REG8, 5, 0), "y=5 should give L");
    ASSERT_EQ(HLI, t_lookup(Y_REG8, 6, 0), "y=6 should give (HL)");
    ASSERT_EQ(A, t_lookup(Y_REG8, 7, 0), "y=7 should give A");
}

void test_t_lookup_z_reg8(void)
{
    TEST_CASE("test_t_lookup_z_reg8");

    // Z_REG8 token should look up 8-bit registers by z field
    ASSERT_EQ(B, t_lookup(Z_REG8, 0, 0), "z=0 should give B");
    ASSERT_EQ(C, t_lookup(Z_REG8, 0, 1), "z=1 should give C");
    ASSERT_EQ(D, t_lookup(Z_REG8, 0, 2), "z=2 should give D");
    ASSERT_EQ(E, t_lookup(Z_REG8, 0, 3), "z=3 should give E");
    ASSERT_EQ(H, t_lookup(Z_REG8, 0, 4), "z=4 should give H");
    ASSERT_EQ(L, t_lookup(Z_REG8, 0, 5), "z=5 should give L");
    ASSERT_EQ(HLI, t_lookup(Z_REG8, 0, 6), "z=6 should give (HL)");
    ASSERT_EQ(A, t_lookup(Z_REG8, 0, 7), "z=7 should give A");
}

void test_t_lookup_y_cc(void)
{
    TEST_CASE("test_t_lookup_y_cc");

    // Y_CC token should look up condition codes
    ASSERT_EQ(NZ, t_lookup(Y_CC, 0, 0), "y=0 should give NZ");
    ASSERT_EQ(Z, t_lookup(Y_CC, 1, 0), "y=1 should give Z");
    ASSERT_EQ(NC, t_lookup(Y_CC, 2, 0), "y=2 should give NC");
    ASSERT_EQ(C, t_lookup(Y_CC, 3, 0), "y=3 should give C");
    ASSERT_EQ(PO, t_lookup(Y_CC, 4, 0), "y=4 should give PO");
    ASSERT_EQ(PE, t_lookup(Y_CC, 5, 0), "y=5 should give PE");
    ASSERT_EQ(P, t_lookup(Y_CC, 6, 0), "y=6 should give P");
    ASSERT_EQ(M, t_lookup(Y_CC, 7, 0), "y=7 should give M");
}

void test_t_lookup_y_alu(void)
{
    TEST_CASE("test_t_lookup_y_alu");

    // Y_ALU token should look up ALU operations
    ASSERT_EQ(ADD, t_lookup(Y_ALU, 0, 0), "y=0 should give ADD");
    ASSERT_EQ(ADC_, t_lookup(Y_ALU, 1, 0), "y=1 should give ADC");
    ASSERT_EQ(SUB, t_lookup(Y_ALU, 2, 0), "y=2 should give SUB");
    ASSERT_EQ(SBC, t_lookup(Y_ALU, 3, 0), "y=3 should give SBC");
    ASSERT_EQ(AND, t_lookup(Y_ALU, 4, 0), "y=4 should give AND");
    ASSERT_EQ(XOR, t_lookup(Y_ALU, 5, 0), "y=5 should give XOR");
    ASSERT_EQ(OR, t_lookup(Y_ALU, 6, 0), "y=6 should give OR");
    ASSERT_EQ(CP, t_lookup(Y_ALU, 7, 0), "y=7 should give CP");
}

void test_t_lookup_p_reg16(void)
{
    TEST_CASE("test_t_lookup_p_reg16");

    // P_REG16 token should look up 16-bit registers by p field
    ASSERT_EQ(BC, t_lookup(P_REG16, 0, 0), "y=0 (p=0) should give BC");
    ASSERT_EQ(BC, t_lookup(P_REG16, 1, 0), "y=1 (p=0) should give BC");
    ASSERT_EQ(DE, t_lookup(P_REG16, 2, 0), "y=2 (p=1) should give DE");
    ASSERT_EQ(DE, t_lookup(P_REG16, 3, 0), "y=3 (p=1) should give DE");
    ASSERT_EQ(HL, t_lookup(P_REG16, 4, 0), "y=4 (p=2) should give HL");
    ASSERT_EQ(HL, t_lookup(P_REG16, 5, 0), "y=5 (p=2) should give HL");
    ASSERT_EQ(SP_, t_lookup(P_REG16, 6, 0), "y=6 (p=3) should give SP");
    ASSERT_EQ(SP_, t_lookup(P_REG16, 7, 0), "y=7 (p=3) should give SP");
}

void test_t_lookup_q_inc_dec(void)
{
    TEST_CASE("test_t_lookup_q_inc_dec");

    // Q_INC_DEC should return INC when q=0, DEC when q=1
    // q is bit 0 of y (y & 1)
    ASSERT_EQ(INC, t_lookup(Q_INC_DEC, 0, 0), "y=0 (q=0) should give INC");
    ASSERT_EQ(DEC, t_lookup(Q_INC_DEC, 1, 0), "y=1 (q=1) should give DEC");
    ASSERT_EQ(INC, t_lookup(Q_INC_DEC, 2, 0), "y=2 (q=0) should give INC");
    ASSERT_EQ(DEC, t_lookup(Q_INC_DEC, 3, 0), "y=3 (q=1) should give DEC");
    ASSERT_EQ(INC, t_lookup(Q_INC_DEC, 4, 0), "y=4 (q=0) should give INC");
    ASSERT_EQ(DEC, t_lookup(Q_INC_DEC, 5, 0), "y=5 (q=1) should give DEC");
    ASSERT_EQ(INC, t_lookup(Q_INC_DEC, 6, 0), "y=6 (q=0) should give INC");
    ASSERT_EQ(DEC, t_lookup(Q_INC_DEC, 7, 0), "y=7 (q=1) should give DEC");
}

void test_t_lookup_literal_passthrough(void)
{
    TEST_CASE("test_t_lookup_literal_passthrough");

    // Literal tokens should pass through unchanged
    ASSERT_EQ(LD, t_lookup(LD, 0, 0), "LD should pass through");
    ASSERT_EQ(NOP, t_lookup(NOP, 0, 0), "NOP should pass through");
    ASSERT_EQ(A, t_lookup(A, 0, 0), "A should pass through");
}

void test_decode_halt(void)
{
    TEST_CASE("test_decode_halt");

    // Opcode 0x76 should decode to HALT
    instr_t i = decode(0, 0x76);
    ASSERT_EQ(HALT_, i.op, "0x76 should decode to HALT");
    ASSERT_EQ(_, i.p1, "HALT should have no p1");
    ASSERT_EQ(_, i.p2, "HALT should have no p2");
}

void test_decode_nop(void)
{
    TEST_CASE("test_decode_nop");

    // Opcode 0x00 should decode to NOP
    instr_t i = decode(0, 0x00);
    if (i.op != NOP) {
        printf("\nDEBUG: Expected NOP (%d/0x%x), got %d (0x%x)\n", NOP, NOP, i.op, i.op);
    }
    ASSERT_EQ(NOP, i.op, "0x00 should decode to NOP");
}

void test_decode_ld_reg_reg(void)
{
    TEST_CASE("test_decode_ld_reg_reg");

    // Test LD B,C (0x41)
    // x=1, y=0 (B), z=1 (C)
    instr_t i = decode(0, 0x41);
    ASSERT_EQ(LD, i.op, "0x41 should be LD");
    ASSERT_EQ(B, i.p1, "0x41 p1 should be B");
    ASSERT_EQ(C, i.p2, "0x41 p2 should be C");

    // Test LD A,A (0x7F)
    // x=1, y=7 (A), z=7 (A)
    i = decode(0, 0x7F);
    ASSERT_EQ(LD, i.op, "0x7F should be LD");
    ASSERT_EQ(A, i.p1, "0x7F p1 should be A");
    ASSERT_EQ(A, i.p2, "0x7F p2 should be A");
}

void test_decode_cb_rlc(void)
{
    TEST_CASE("test_decode_cb_rlc");

    // RLC B (0xCB 0x00)
    // x=0, y=0 (RLC), z=0 (B)
    instr_t i = decode(0xCB, 0x00);
    ASSERT_EQ(RLC, i.op, "CB 00 should be RLC");
    ASSERT_EQ(B, i.p1, "CB 00 p1 should be B");
}

void test_decode_cb_bit(void)
{
    TEST_CASE("test_decode_cb_bit");

    // BIT 0,B (0xCB 0x40)
    // x=1, y=0, z=0
    instr_t i = decode(0xCB, 0x40);
    ASSERT_EQ(BIT, i.op, "CB 40 should be BIT");
    ASSERT_EQ(0, i.p1, "CB 40 p1 should be 0 (digit)");
    ASSERT_EQ(B, i.p2, "CB 40 p2 should be B");

    // BIT 7,A (0xCB 0x7F)
    // x=1, y=7, z=7
    i = decode(0xCB, 0x7F);
    ASSERT_EQ(BIT, i.op, "CB 7F should be BIT");
    ASSERT_EQ(7, i.p1, "CB 7F p1 should be 7 (digit)");
    ASSERT_EQ(A, i.p2, "CB 7F p2 should be A");
}

int main(void)
{
    TEST_INIT();

    TEST_SUITE("Lookup Function Tests");

    test_t_lookup_y_reg8();
    test_t_lookup_z_reg8();
    test_t_lookup_y_cc();
    test_t_lookup_y_alu();
    test_t_lookup_p_reg16();
    test_t_lookup_q_inc_dec();
    test_t_lookup_literal_passthrough();

    TEST_SUITE("Decode Function Tests");

    test_decode_halt();
    test_decode_nop();
    test_decode_ld_reg_reg();
    test_decode_cb_rlc();
    test_decode_cb_bit();

    TEST_REPORT();
}
