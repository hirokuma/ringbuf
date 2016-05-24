TARGET = tst
OBJS = \
	gtest_ringbuf.o

OBJS += \
	gtest_main.o

LCOV_DIR = _lcov

GTEST_ROOT = ../../..
GTEST_DIR = $(GTEST_ROOT)/googletest
GMOCK_DIR = $(GTEST_ROOT)/googlemock

CXXFLAGS = -Wall -I$(GTEST_DIR)/include -I$(GMOCK_DIR)/include
LDOPT = $(GTEST_ROOT)/libgmock.a -coverage

CXXFLAGS += -I.. -coverage

all: .Depend $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) -pthread $(OBJS) $(LDOPT)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $<

clean:
	$(RM) -r .Depend $(OBJS) $(TARGET) *.gcda *.gcno $(LCOV_DIR)

SRCS = $(OBJS:.o=.cpp)

.Depend:
	$(CXX) $(CXXFLAGS) -MM $(SRCS) > .Depend

lcov:
	@mkdir -p $(LCOV_DIR)
	lcov -c -d . -o $(LCOV_DIR)/lcov.info
	genhtml -o $(LCOV_DIR)/lcovhtml $(LCOV_DIR)/lcov.info

include .Depend
