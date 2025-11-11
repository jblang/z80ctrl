# Makefile for Z80 disassembler unit tests
# Compiles for host system (not AVR)

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g -I. -I./mocks -I.. -Imocks/util -Imocks/hardware
LDFLAGS =

# Source files from parent directory
DISASM_SOURCES = \
	../literals.c \
	../immediate.c \
	../lookup.c \
	../disasm.c

# Mock implementations
MOCK_SOURCES = \
	mocks/hardware/memory.c

# Test framework
FRAMEWORK_SOURCES = \
	test_framework.c

# Test executables
TEST_BINS = \
	test_literals \
	test_immediate \
	test_regsub \
	test_lookup \
	test_disasm \
	test_comprehensive \
	test_cb_ed \
	test_ix_iy

# Object files
DISASM_OBJS = $(DISASM_SOURCES:.c=.o)
MOCK_OBJS = $(MOCK_SOURCES:.c=.o)
FRAMEWORK_OBJS = $(FRAMEWORK_SOURCES:.c=.o)

.PHONY: all clean test

all: $(TEST_BINS)

# Build each test binary
test_literals: test_literals.o $(DISASM_OBJS) $(MOCK_OBJS) $(FRAMEWORK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test_immediate: test_immediate.o $(DISASM_OBJS) $(MOCK_OBJS) $(FRAMEWORK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test_regsub: test_regsub.o $(FRAMEWORK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test_lookup: test_lookup.o $(DISASM_OBJS) $(MOCK_OBJS) $(FRAMEWORK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test_disasm: test_disasm.o $(DISASM_OBJS) $(MOCK_OBJS) $(FRAMEWORK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test_comprehensive: test_comprehensive.o $(DISASM_OBJS) $(MOCK_OBJS) $(FRAMEWORK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test_cb_ed: test_cb_ed.o $(DISASM_OBJS) $(MOCK_OBJS) $(FRAMEWORK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test_ix_iy: test_ix_iy.o $(DISASM_OBJS) $(MOCK_OBJS) $(FRAMEWORK_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Run all tests
test: $(TEST_BINS)
	@echo "Running all tests..."
	@for test in $(TEST_BINS); do \
		echo ""; \
		echo "========================================"; \
		echo "Running $$test"; \
		echo "========================================"; \
		./$$test || exit 1; \
	done
	@echo ""
	@echo "========================================"
	@echo "All tests passed!"
	@echo "========================================"

clean:
	rm -f $(TEST_BINS) test_*.o $(DISASM_OBJS) $(MOCK_OBJS) $(FRAMEWORK_OBJS)
	rm -f ../*.o mocks/*.o mocks/hardware/*.o *.o

.SUFFIXES: .c .o
