# standard_defs.mk
CXX := c++
DSYM := dsymutil
#CXXFLAGS := -std=c++11 -O0 -g
CXXFLAGS := -std=c++11 -O2 -framework CoreFoundation

INCLUDE_DIRS := -I../../src -I../../include -I../../include/osx
LDFLAGS := -L../../lib/osx
LIBS := -ltbb -lstxxl_debug
LD  := link