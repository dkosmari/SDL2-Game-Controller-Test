SUBDIRS = sdl2xx


AM_CXXFLAGS = -Wall -Wextra -Werror

AM_CPPFLAGS = \
	-I$(srcdir)/imgui

noinst_LTLIBRARIES = \
	imgui/libimgui.la \
	implot/libimplot.la


# imgui

imgui_libimgui_la_SOURCES = \
	custom_imconfig.h \
	imgui/backends/imgui_impl_sdl2.cpp \
	imgui/backends/imgui_impl_sdl2.h \
	imgui/backends/imgui_impl_sdlrenderer2.cpp \
	imgui/backends/imgui_impl_sdlrenderer2.h \
	imgui/imconfig.h \
	imgui/imgui.cpp \
	imgui/imgui_demo.cpp \
	imgui/imgui_draw.cpp \
	imgui/imgui.h \
	imgui/imgui_internal.h \
	imgui/imgui_tables.cpp \
	imgui/imgui_widgets.cpp \
	imgui/imstb_rectpack.h \
	imgui/imstb_textedit.h \
	imgui/misc/cpp/imgui_stdlib.cpp \
	imgui/misc/cpp/imgui_stdlib.h \
	imgui/misc/freetype/imgui_freetype.cpp \
	imgui/misc/freetype/imgui_freetype.h

imgui_libimgui_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(SDL_CFLAGS) \
	$(FT_CFLAGS) \
	-I$(srcdir)/imgui \
	-DIMGUI_USER_CONFIG='"$(srcdir)/custom_imconfig.h"'


# implot

implot_libimplot_la_SOURCES = \
	implot/implot.cpp \
	implot/implot.h \
	implot/implot_internal.h \
	implot/implot_items.cpp

implot_libimplot_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(srcdir)/imgui \
	-DIMGUI_USER_CONFIG='"$(srcdir)/custom_imconfig.h"'

if ENABLE_WIIU

implot_libimplot_la_CPPFLAGS += -Dtimegm=mktime

endif ENABLE_WIIU


.PHONY: company
company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt
