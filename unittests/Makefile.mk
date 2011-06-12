check_PROGRAMS += unittest

TESTS += unittest$(EXEEXT)

# Source files holding tests.
unittest_SOURCES = \
 unittests/collection.cc \
 unittests/docdata.cc \
 unittests/doctojson.cc \
 unittests/jsonmanip/conditionals.cc \
 unittests/jsonmanip/mapping.cc \
 unittests/jsonmanip/walker.cc \
 unittests/ngramcat/categoriser.cc \
 unittests/ngramcat/profile.cc \
 unittests/pipe.cc \
 unittests/schema.cc \
 unittests/search.cc \
 unittests/threadsafequeue.cc

unittest_SOURCES += \
 unittests/runner.cc

unittest_LDADD = \
 libserver.a \
 libhttpserver.a \
 librest.a \
 libjsonxapian.a \
 libngramcat.a \
 libjsonmanip.a \
 libcjktokenizer.a \
 libdbgroup.a \
 libutils.a \
 libjsoncpp.a \
 libunittestpp.a \
 liblogger.a \
 libmatchspies.a \
 libxapiancommon.a \
 libs/libmicrohttpd/src/daemon/libmicrohttpd.la \
 $(XAPIAN_LIBS)

unittest_LDFLAGS = \
 -pthread
