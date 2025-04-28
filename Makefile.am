SUBDIRS = \
	$(subdirs) \
	$(subdirs_extra) \
	.


AM_CPPFLAGS = $(SDL2_CFLAGS)

LIBS = $(SDL2_LIBS)

AM_CXXFLAGS = -Wall -Wextra -Werror


noinst_LIBRARIES = \
	libimgui.a \
	libimplot.a


libimgui_a_SOURCES = \
	external/imgui/backends/imgui_impl_sdl2.cpp \
	external/imgui/backends/imgui_impl_sdl2.h \
	external/imgui/backends/imgui_impl_sdlrenderer2.cpp \
	external/imgui/backends/imgui_impl_sdlrenderer2.h \
	external/imgui/imconfig.h \
	external/imgui/imgui.cpp \
	external/imgui/imgui_draw.cpp \
	external/imgui/imgui.h \
	external/imgui/imgui_internal.h \
	external/imgui/imgui_tables.cpp \
	external/imgui/imgui_widgets.cpp \
	external/imgui/imstb_rectpack.h \
	external/imgui/imstb_textedit.h \
	external/imgui/imstb_truetype.h

libimgui_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-Iexternal/imgui


libimplot_a_SOURCES = \
	external/implot/implot.cpp \
	external/implot/implot.h \
	external/implot/implot_internal.h \
	external/implot/implot_items.cpp

libimplot_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-Iexternal/imgui


bin_PROGRAMS = sdl2-gamecontroller-test

sdl2_gamecontroller_test_SOURCES = \
	src/App.cpp src/App.hpp \
	src/JoystickListWindow.cpp src/JoystickListWindow.hpp \
	src/JoystickWindow.cpp src/JoystickWindow.hpp \
	src/main.cpp \
	src/Window.hpp

sdl2_gamecontroller_test_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-Iexternal/sdl2xx/include \
	-Iexternal/imgui \
	-Iexternal/implot

sdl2_gamecontroller_test_LDADD = \
	libimplot.a \
	libimgui.a \
	external/sdl2xx/libsdl2xx.a


.PHONY: company
company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt
