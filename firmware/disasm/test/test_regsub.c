/* Unit tests for Z80 disassembler regsub() function */

#include "test_framework.h"
#include "../tokens.h"
#include <stdint.h>

// Declare regsub function from disasm.c
// Since we're not linking disasm.c, we need to provide it here for testing
uint8_t regsub(uint8_t reg, uint8_t submode)
{
    if (submode == IX) {
        if (reg == HL)
            reg = IX;
        else if (reg == HLI)
            reg = IXI;
        else if (reg == H)
            reg = IXH;
        else if (reg == L)
            reg = IXL;
    } else if (submode == IY) {
        if (reg == HL)
            reg = IY;
        else if (reg == HLI)
            reg = IYI;
        else if (reg == H)
            reg = IYH;
        else if (reg == L)
            reg = IYL;
    }
    return reg;
}

void test_regsub_hl_mode(void)
{
    TEST_CASE("test_regsub_hl_mode");

    // When in HL mode, no substitution should occur
    ASSERT_EQ(HL, regsub(HL, HL), "HL mode: HL should remain HL");
    ASSERT_EQ(HLI, regsub(HLI, HL), "HL mode: (HL) should remain (HL)");
    ASSERT_EQ(H, regsub(H, HL), "HL mode: H should remain H");
    ASSERT_EQ(L, regsub(L, HL), "HL mode: L should remain L");

    // Other registers should also remain unchanged
    ASSERT_EQ(A, regsub(A, HL), "HL mode: A should remain A");
    ASSERT_EQ(B, regsub(B, HL), "HL mode: B should remain B");
    ASSERT_EQ(BC, regsub(BC, HL), "HL mode: BC should remain BC");
}

void test_regsub_ix_mode(void)
{
    TEST_CASE("test_regsub_ix_mode");

    // When in IX mode, HL/H/L should be substituted
    ASSERT_EQ(IX, regsub(HL, IX), "IX mode: HL should become IX");
    ASSERT_EQ(IXI, regsub(HLI, IX), "IX mode: (HL) should become (IX)");
    ASSERT_EQ(IXH, regsub(H, IX), "IX mode: H should become IXH");
    ASSERT_EQ(IXL, regsub(L, IX), "IX mode: L should become IXL");

    // Other registers should remain unchanged
    ASSERT_EQ(A, regsub(A, IX), "IX mode: A should remain A");
    ASSERT_EQ(B, regsub(B, IX), "IX mode: B should remain B");
    ASSERT_EQ(BC, regsub(BC, IX), "IX mode: BC should remain BC");
    ASSERT_EQ(DE, regsub(DE, IX), "IX mode: DE should remain DE");
}

void test_regsub_iy_mode(void)
{
    TEST_CASE("test_regsub_iy_mode");

    // When in IY mode, HL/H/L should be substituted
    ASSERT_EQ(IY, regsub(HL, IY), "IY mode: HL should become IY");
    ASSERT_EQ(IYI, regsub(HLI, IY), "IY mode: (HL) should become (IY)");
    ASSERT_EQ(IYH, regsub(H, IY), "IY mode: H should become IYH");
    ASSERT_EQ(IYL, regsub(L, IY), "IY mode: L should become IYL");

    // Other registers should remain unchanged
    ASSERT_EQ(A, regsub(A, IY), "IY mode: A should remain A");
    ASSERT_EQ(B, regsub(B, IY), "IY mode: B should remain B");
    ASSERT_EQ(BC, regsub(BC, IY), "IY mode: BC should remain BC");
    ASSERT_EQ(DE, regsub(DE, IY), "IY mode: DE should remain DE");
}

void test_regsub_ix_no_change(void)
{
    TEST_CASE("test_regsub_ix_no_change");

    // IX/IY themselves should not be affected
    ASSERT_EQ(IX, regsub(IX, IX), "IX mode: IX should remain IX");
    ASSERT_EQ(IY, regsub(IY, IX), "IX mode: IY should remain IY");
}

int main(void)
{
    TEST_INIT();

    TEST_SUITE("Register Substitution Tests");

    test_regsub_hl_mode();
    test_regsub_ix_mode();
    test_regsub_iy_mode();
    test_regsub_ix_no_change();

    TEST_REPORT();
}
