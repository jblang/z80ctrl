#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

/* Minimal test framework for Z80 disassembler tests */

#include <stdio.h>
#include <string.h>

// Test statistics
extern int tests_run;
extern int tests_passed;
extern int tests_failed;

// Initialize test framework
#define TEST_INIT() do { \
    tests_run = 0; \
    tests_passed = 0; \
    tests_failed = 0; \
} while(0)

// Report test results
#define TEST_REPORT() do { \
    printf("\n========================================\n"); \
    printf("Tests run: %d\n", tests_run); \
    printf("Tests passed: %d\n", tests_passed); \
    printf("Tests failed: %d\n", tests_failed); \
    printf("========================================\n"); \
    return (tests_failed == 0) ? 0 : 1; \
} while(0)

// Assert macros
#define ASSERT(cond, msg) do { \
    tests_run++; \
    if (cond) { \
        tests_passed++; \
        printf("."); \
        fflush(stdout); \
    } else { \
        tests_failed++; \
        printf("\n[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
    } \
} while(0)

#define ASSERT_EQ(expected, actual, msg) do { \
    tests_run++; \
    if ((expected) == (actual)) { \
        tests_passed++; \
        printf("."); \
        fflush(stdout); \
    } else { \
        tests_failed++; \
        printf("\n[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
        printf("  Expected: %d (0x%x)\n", (int)(expected), (unsigned)(expected)); \
        printf("  Actual:   %d (0x%x)\n", (int)(actual), (unsigned)(actual)); \
    } \
} while(0)

#define ASSERT_STR_EQ(expected, actual, msg) do { \
    tests_run++; \
    if (strcmp((expected), (actual)) == 0) { \
        tests_passed++; \
        printf("."); \
        fflush(stdout); \
    } else { \
        tests_failed++; \
        printf("\n[FAIL] %s:%d: %s\n", __FILE__, __LINE__, msg); \
        printf("  Expected: \"%s\"\n", expected); \
        printf("  Actual:   \"%s\"\n", actual); \
    } \
} while(0)

// Test suite macros
#define TEST_SUITE(name) \
    printf("\n=== Running test suite: %s ===\n", name)

#define TEST_CASE(name) \
    printf("\n%s: ", name)

#endif // TEST_FRAMEWORK_H
