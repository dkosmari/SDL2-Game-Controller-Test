SUBDIRS = sdl2xx


AM_CXXFLAGS = -Wall -Wextra -Werror

AM_CPPFLAGS = \
	$(SDL2_CFLAGS) \
	-Iimgui \
	-Iimplot

LIBS = $(SDL2_LIBS)


noinst_LIBRARIES = \
	libimgui.a \
	libimplot.a


libimgui_a_SOURCES = \
	imgui/backends/imgui_impl_sdl2.cpp \
	imgui/backends/imgui_impl_sdl2.h \
	imgui/backends/imgui_impl_sdlrenderer2.cpp \
	imgui/backends/imgui_impl_sdlrenderer2.h \
	imgui/imconfig.h \
	imgui/imgui.cpp \
	imgui/imgui_draw.cpp \
	imgui/imgui.h \
	imgui/imgui_internal.h \
	imgui/imgui_tables.cpp \
	imgui/imgui_widgets.cpp \
	imgui/imstb_rectpack.h \
	imgui/imstb_textedit.h \
	imgui/imstb_truetype.h \
	imgui/misc/cpp/imgui_stdlib.cpp \
	imgui/misc/cpp/imgui_stdlib.h

libimgui_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-Iimgui


libimplot_a_SOURCES = \
	implot/implot.cpp \
	implot/implot.h \
	implot/implot_internal.h \
	implot/implot_items.cpp

libimplot_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-Iimgui


.PHONY: company
company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt
