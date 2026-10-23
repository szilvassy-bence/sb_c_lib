# c-from-scratch — fordítás és ellenőrzés
#
#   make                   minden program (gcc-13, szigorú warningok)
#   make sb_string         egy program
#   make analyze           statikus analízis — futtatás nélkül
#   make san-sb_string     clang + ASan/UBSan: fordít ÉS futtat
#   make opt-sb_string     -O2 build (az optimalizáló más UB-t hoz elő)
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

# A CC-nek a make-ben van beépített alapértéke (cc), ezért ?= nem hatna rá.
# A := felülírja, a parancssori `make CC=gcc` viszont továbbra is nyer.
CC     := gcc-13
SAN_CC ?= clang

WARN     := -Wall -Wextra -Wpedantic
CFLAGS   := -std=c17 $(WARN) -g -O0

# Header-dependency generálás. Külön változó, NEM a CFLAGS része, mert az
# `analyze` target `-o /dev/null`-lal fordít, és a -MMD onnan a fájlnevet
# származtatja: "fatal error: opening dependency file /dev/null.d".
# -MP: minden headerhez üres szabályt is ír, így egy header törlése nem
#      "No rule to make target" hibát ad, hanem újrafordítást.
DEPFLAGS := -MMD -MP

# -g nélkül az ASan riport csak címeket ad, függvénynév és sorszám nélkül.
SANFLAGS := -std=c17 $(WARN) -g -O0 \
            -fsanitize=address,undefined \
            -fsanitize-address-use-after-return=always

SRCS := $(wildcard *.c)
BINS := $(SRCS:.c=)
DEPS := $(SRCS:.c=.d)

# A `%.h` prerequisite szándékosan lekerült: a header-függést most a .d fájlok
# adják, pontosabban (a tranzitív include-okat is), és így a header nélküli
# .c sem esik át a make beépített szabályára.
%.o: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Csak objektumok linkelése — a tesztfájl is előbb .o lesz a fenti szabállyal.
# Ez azért kell, mert a .d-ből beolvasott header-prerequisite-ek bekerülnének
# a $^-ba, és a `gcc ... sb_str.h -o sb_str_tests` a headert bemenetként
# próbálná feldolgozni. Így a $^ garantáltan csak .o.
%_tests: %_tests.o %.o
	$(CC) $(CFLAGS) $^ -o $@

# A .o-k pattern rule termékei, amiket a make egyébként köztes fájlként
# törölne minden build végén (majd újrafordítana).
.SECONDARY:

TESTS := sb_int_tests sb_str_tests sb_mem_freelist_tests

all: $(TESTS)

test: $(TESTS)
	./sb_int_tests
	./sb_str_tests
	./sb_mem_freelist_tests

# Statikus analízis. Nem állítja meg a többi fájl vizsgálatát az első hibánál.
analyze:
	@for f in $(SRCS); do \
	    echo "== $$f =="; \
	    $(CC) $(CFLAGS) -fanalyzer -c $$f -o /dev/null || true; \
	done

# A futásidejű ellenőrzés csak akkor szól, ha a hibás ág LEFUT.
san-%: %.c
	$(SAN_CC) $(SANFLAGS) -o $*_san $<
	ASAN_OPTIONS=detect_stack_use_after_return=1 ./$*_san

san-%_tests: %_tests.c %.
	$(SAN_CC) $(SANFLAGS) -o $*_tests_san $^

opt-%: %.c
	$(CC) -std=c17 $(WARN) -g -O2 -o $*_opt $<

check: analyze all

clean:
	rm -f $(BINS) *.o *.d *_san *_opt

.PHONY: all analyze check clean

# A generált dependency-k beolvasása. A '-' azért kell, mert az első buildnél
# (és clean után) még nincsenek .d fájlok — hiba helyett csendben kihagyja.
# Ez tanítja meg a make-nek, hogy sb_str.c az sb_int.h-tól is függ, amit a
# fenti `%.o: %.c %.h` szabály önmagában nem tud.
-include $(DEPS)
