SUBDIRS = \
	external \
	.


AM_CXXFLAGS = -Wall -Wextra -Werror

AM_CPPFLAGS = \
	$(SDL2_CFLAGS) \
	-Iexternal/sdl2xx/include \
	-Iexternal/imgui \
	-Iexternal/implot

LIBS = $(SDL2_LIBS)


bin_PROGRAMS = sdl2-gamecontroller-test

sdl2_gamecontroller_test_SOURCES = \
	src/App.cpp src/App.hpp \
	src/GameControllerListWindow.cpp src/GameControllerListWindow.hpp \
	src/GameControllerWindow.cpp src/GameControllerWindow.hpp \
	src/JoystickListWindow.cpp src/JoystickListWindow.hpp \
	src/JoystickWindow.cpp src/JoystickWindow.hpp \
	src/main.cpp \
	src/Window.cpp src/Window.hpp


sdl2_gamecontroller_test_LDADD = \
	external/libimplot.a \
	external/libimgui.a \
	external/sdl2xx/libsdl2xx.a


.PHONY: company
company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt
