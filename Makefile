# c-from-scratch — fordítás és ellenőrzés
#
#   make                   minden program (gcc-13, szigorú warningok)
#   make sb_string         egy program
#   make analyze           statikus analízis — futtatás nélkül
#   make san-sb_string     clang + ASan/UBSan: fordít ÉS futtat
#   make vg-sb_string      valgrind memory analysis
#   make check             analyze + minden build
#   make clean
#
# Munkamegosztás — mérve, nem feltételezve (2026-07-28, dangling pointer teszt):
#
#   gcc-13 -fanalyzer : fordítási időben elkapja a "stale stack frame" hibát,
#                       még VLA-nál is, és futtatni sem kell hozzá.
#                       A gcc 11 analizátora ezt még NEM tudja.
#   clang ASan        : futásidőben, use-after-return detektálással.
#                       Csak azt látja, ami ténylegesen lefut, viszont
#                       megnevezi a keretet és a pontos címet.
#
# Kiegészítik egymást: a statikus olyan ágat is lát, amit sosem futtatsz,
# a futásidejű viszont gyakorlatilag nem ad téves riasztást.

DIR_BUILD      := build
DIR_BUILD_TEST := $(DIR_BUILD)/test
DIR_BUILD_SAN  := $(DIR_BUILD)/san
DIR_BUILD_OBJS := $(DIR_BUILD)/obj
DIR_SRCS       := src
DIR_SRC_TEST   := test

CC     := gcc-13
SAN_CC ?= clang

WARN     := -Wall -Wextra -Wpedantic
CFLAGS   := -std=c17 $(WARN) -g -O0 -Isrc

DEPFLAGS := -MMD -MP

SANFLAGS := -std=c17 $(WARN) -g -O0 \
            -fsanitize=address,undefined \
            -fsanitize-address-use-after-return=always -Isrc

SRCS      := $(wildcard $(DIR_SRCS)/*.c)
TEST_SRCS := $(wildcard $(DIR_SRC_TEST)/*.c)
OBJS  := $(patsubst $(DIR_SRCS)/%.c,$(DIR_BUILD_OBJS)/src/%.o,$(SRCS)) $(patsubst $(DIR_SRC_TEST)/%.c,$(DIR_BUILD_OBJS)/test/%.o,$(TEST_SRCS))
DEPS := $(OBJS:.o=.d)

TEST_TARGETS := $(patsubst $(DIR_SRC_TEST)/%.c,$(DIR_BUILD_TEST)/%,$(TEST_SRCS))

$(DIR_BUILD_OBJS)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(DIR_BUILD_TEST)/%: $(DIR_BUILD_OBJS)/test/%.o $(DIR_BUILD_OBJS)/src/%.o
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@

.SECONDARY:

all: $(TEST_TARGETS)

test: $(TEST_TARGETS)
	@for t in $(TEST_TARGETS); do ./$$t || exit 1; done

analyze:
	@for f in $(SRCS); do \
	    echo "== $$f =="; \
	    $(CC) $(CFLAGS) -fanalyzer -c $$f -o /dev/null || true; \
	done

$(DIR_BUILD_TEST)/sb_str $(DIR_BUILD_TEST)/sb_mem_arena: $(DIR_BUILD_OBJS)/src/sb_int.o

san-%: $(DIR_SRC_TEST)/%.c $(DIR_SRCS)/%.c
	mkdir -p $(DIR_BUILD_SAN)
	$(SAN_CC) $(SANFLAGS) $^ -o $(DIR_BUILD_SAN)/$*
	./$(DIR_BUILD_SAN)/$*

vg-%: $(DIR_SRC_TEST)/%.c $(DIR_SRCS)/%.c $(DIR_SRCS)/sb_int.c
	mkdir -p $(DIR_BUILD)/vg
	gcc $(CFLAGS) $^ -o $(DIR_BUILD)/vg/$*
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=9 $(DIR_BUILD)/vg/$*

check: analyze all

clean:
	rm -rf $(DIR_BUILD)

.PHONY: all analyze check clean

-include $(DEPS)
