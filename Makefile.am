EXTRA_DIST = \
	assets \
	bootstrap \
	COPYING.GPLv3 \
	README.md


SUBDIRS = external


external/%:
	$(MAKE) $(AM_MAKEFLAGS) -C external


AM_CXXFLAGS = -Wall -Wextra -Werror

AM_CPPFLAGS = \
	$(SDL_CFLAGS) \
	$(FT_CFLAGS) \
	-I$(srcdir)/external/sdl2xx/include \
	-I$(srcdir)/external/imgui \
	-I$(srcdir)/external/implot \
	-DIMGUI_USER_CONFIG='"$(srcdir)/external/custom_imconfig.h"'

LDADD = \
	external/imgui/libimgui.la \
	external/implot/libimplot.la \
	external/sdl2xx/lib/libsdl2xx.la \
	$(SDL_LIBS) \
	$(FT_LIBS)


BIN_FILE := sdl2-gamecontroller-test.elf


bin_PROGRAMS = $(BIN_FILE)

sdl2_gamecontroller_test_elf_SOURCES = \
	src/App.cpp \
	src/App.hpp \
	src/GameControllerListWindow.cpp \
	src/GameControllerListWindow.hpp \
	src/GameControllerWindow.cpp \
	src/GameControllerWindow.hpp \
	src/JoystickListWindow.cpp \
	src/JoystickListWindow.hpp \
	src/JoystickWindow.cpp \
	src/JoystickWindow.hpp \
	src/main.cpp \
	src/UI.cpp \
	src/UI.hpp \
	src/Window.cpp \
	src/Window.hpp \
	src/wiiu-stderr.cpp \
	src/wiiu-stdout.cpp


.PHONY: company run wiiu-install wiiu-uninstall


if ENABLE_WIIU

# FIX: implot wants to use timegm
AM_CPPFLAGS += -Dtimegm=mktime

RPX_FILE  = $(BIN_FILE:.elf=.rpx)
WUHB_FILE = $(RPX_FILE:.rpx=.wuhb)

$(WUHB_FILE): $(RPX_FILE)
	$(WUHBTOOL) \
		$< $@ \
		--content=$(srcdir)/assets/content \
		--name="$(PACKAGE_NAME)" \
		--short-name="$(PACKAGE)" \
		--author="Daniel K. O."


run: all
	WIILOAD=tcp:wiiu wiiload $(WUHB_FILE)


wiiu-install: all
	curl --upload-file $(WUHB_FILE) "ftp://wiiu:/fs/vol/external01/wiiu/apps/"


wiiu-uninstall:
	curl "ftp://wiiu" --quote "DELE /fs/vol/external01/wiiu/apps/$(WUHB_FILE)"


all-local: $(WUHB_FILE)

else !ENABLE_WIIU

run: all
	cd $(top_srcdir) && $(abs_builddir)/$(BIN_FILE)

endif !ENABLE_WIIU


@INC_AMINCLUDE@


company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt
