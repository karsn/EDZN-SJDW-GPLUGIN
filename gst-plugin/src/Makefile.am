# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstplugin.la to something else, e.g. libmysomething.la     #
##############################################################################
#plugin_LTLIBRARIES = libgstqreader.la libgsthybridbin.la
plugin_LTLIBRARIES = libgstqreader.la libgsthybridbin.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

## Plugin 1

# sources used to compile this plug-in
libgstqreader_la_SOURCES = gstqreader.c gstqreader.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstqreader_la_CFLAGS = $(GST_CFLAGS)
libgstqreader_la_LIBADD = $(GST_LIBS)
libgstqreader_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstqreader_la_LIBTOOLFLAGS = --tag=disable-static

## Plugin 2 (audio filter example)

# sources used to compile this plug-in
libgsthybridbin_la_SOURCES = gsthybridbin.cpp gsthybridbin.h ImageReaderSource.cpp ImageReaderSource.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgsthybridbin_la_CFLAGS = $(GST_CFLAGS)
libgsthybridbin_la_CXXFLAGS = $(GST_CFLAGS)
libgsthybridbin_la_LIBADD = $(GST_LIBS)
libgsthybridbin_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgsthybridbin_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstqreader.h gsthybridbin.h ImageReaderSource.h
