# standard_defs.mk
CXX := c++
CXXFLAGS := -O2 -std=c++11

INCLUDE_DIRS := -I../../src -I../../include -I../../include/osx
LDFLAGS := -L../../lib/osx
LIBS := -ltbb -lstxxl_debug
LD  := link