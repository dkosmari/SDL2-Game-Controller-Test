AM_CPPFLAGS = $(SDL2_CFLAGS)

LIBS = $(SDL2_LIBS)

AM_CXXFLAGS = -Wall -Wextra -Werror


bin_PROGRAMS = sdl2-gamecontroller-test

sdl2_gamecontroller_test_SOURCES = \
	src/main.cpp



.PHONY: company
company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt
