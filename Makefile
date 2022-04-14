## executables
EXES=bin/adventure/main-adventure bin/jump/main-jump
run-adventure: bin/adventure/main-adventure
	$<
run-jump: bin/jump/main-jump
	$<
run-adventure_d: bin/adventure/main-adventure_d
	$<
run-jump_d: bin/jump/main-jump_d
	$<



## location of cxxtest
CXXTEST_HOME?=extern/cxxtest-4.4


## various options for possible build configurations
## comment in one of the following sets or add your own

CXXFLAGS_BASE=
LDFLAGS_BASE=

## for gprof
#CXXFLAGS_BASE:=$(CXXFLAGS_BASE) -O2 -pg
#LDFLAGS_BASE:=$(LDFLAGS_BASE) -pg

## the following should not need to change

## generic options
CXXFLAGS_BASE:=$(CXXFLAGS_BASE) -std=c++17 -Wall -Werror -pedantic-errors -Iinclude -Isrc
LDFLAGS_BASE:=$(LDFLAGS_BASE) -std=c++17

## platform-specific options
ifeq ($(OS),Windows_NT)
	CXXFLAGS_BASE:=$(CXXFLAGS_BASE) -I/mingw64/include/SDL2
	LDFLAGS_BASE:=$(LDFLAGS_BASE) -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
	DOXYGEN=doxygen
else ifeq ($(shell sh -c 'uname'),Darwin)
	CXXFLAGS_BASE:=$(CXXFLAGS_BASE) -F/Library/Frameworks -I/Library/Frameworks/SDL2.framework/Headers -I/Library/Frameworks/SDL2_image.framework/Headers -I/Library/Frameworks/SDL2_mixer.framework/Headers -I/Library/Frameworks/SDL2_ttf.framework/Headers
	LDFLAGS_BASE:=$(LDFLAGS_BASE) -F/Library/Frameworks -framework SDL2 -framework SDL2_image -framework SDL2_mixer -framework SDL2_ttf
	ifneq ($(wildcard /Applications/Doxygen.app/Contents/Resources/doxygen),)
		DOXYGEN=/Applications/Doxygen.app/Contents/Resources/doxygen
	else
		DOXYGEN=doxygen
	endif
else
	CXXFLAGS_BASE:=$(CXXFLAGS_BASE) $(shell sh -c 'sdl2-config --cflags')
	LDFLAGS_BASE:=$(LDFLAGS_BASE) $(shell sh -c 'sdl2-config --libs') -lSDL2_image -lSDL2_mixer -lSDL2_ttf
	DOXYGEN=doxygen
endif

## files
EXES_DEBUG=$(addsuffix _d,$(EXES))
EXECUTABLE_SOURCE_FILES=$(shell sh -c '/usr/bin/find src -name "main*.cpp" 2>/dev/null')
SOURCE_FILES=$(filter-out $(EXECUTABLE_SOURCE_FILES),$(shell sh -c '/usr/bin/find src -name "*.cpp" 2>/dev/null'))
HEADER_FILES=$(shell sh -c '/usr/bin/find include src -name "*.hpp" 2>/dev/null')
OBJECT_FILES_RELEASE=$(SOURCE_FILES:src/%.cpp=build/exe/%.o)
OBJECT_FILES_DEBUG=$(SOURCE_FILES:src/%.cpp=build/exe/%_d.o)
EXECUTABLES_RELEASE=$(patsubst src/%.cpp,bin/%,$(EXECUTABLE_SOURCE_FILES))
EXECUTABLES_DEBUG=$(patsubst src/%.cpp,bin/%_d,$(EXECUTABLE_SOURCE_FILES))

## test files
CXXTEST_GEN=$(CXXTEST_HOME)/bin/cxxtestgen
CXXTEST_INCLUDE=$(CXXTEST_HOME)

TEST_FILES=$(shell sh -c '/usr/bin/find test -name "*.cxxtest.hpp"')
TEST_SOURCE_FILES=$(TEST_FILES:test/%.cxxtest.hpp=build/test/%.cxxtest.cpp) build/test/runner.cpp
TEST_OBJECT_FILES_RELEASE=$(TEST_SOURCE_FILES:.cpp=.o)
TEST_OBJECT_FILES_DEBUG=$(TEST_SOURCE_FILES:.cpp=_d.o)

CXXFLAGS_BASE:=$(CXXFLAGS_BASE) -I$(CXXTEST_INCLUDE)

## for optimization
CXXFLAGS_RELEASE:=$(CXXFLAGS_BASE) -O2 -DNDEBUG
LDFLAGS_RELEASE:=$(LDFLAGS_BASE)

## for gdb or valgrind
CXXFLAGS_DEBUG:=$(CXXFLAGS_BASE) -Og -ggdb -fno-inline -fno-omit-frame-pointer -D_DEBUG
LDFLAGS_DEBUG:=$(LDFLAGS_BASE)



## rules
.PHONY: all clean doc exe exe_d tests tests_d run run_d run-tests run-tests_d

all: doc exe exe_d tests tests_d



doc: $(HEADER_FILES) $(SOURCE_FILES) Makefile
	$(DOXYGEN)

exe: $(EXECUTABLES_RELEASE)

exe_d: $(EXECUTABLES_DEBUG)

tests: bin/test

tests_d: bin/test_d



run-tests: bin/test
	bin/test

run-tests_d: bin/test_d
	bin/test_d



$(EXECUTABLES_RELEASE): bin/%: build/exe/%.o $(OBJECT_FILES_RELEASE)
	mkdir -p $(dir $@)
	g++  $^ $(LDFLAGS_RELEASE) -o $@

$(EXECUTABLES_DEBUG): bin/%: build/exe/%.o $(OBJECT_FILES_DEBUG)
	mkdir -p $(dir $@)
	g++  $^ $(LDFLAGS_DEBUG) -o $@

build/exe/%.o: src/%.cpp $(HEADER_FILES) Makefile
	mkdir -p $(dir $@)
	g++ -c $< $(CXXFLAGS_RELEASE) -o $@

build/exe/%_d.o: src/%.cpp $(HEADER_FILES) Makefile
	mkdir -p $(dir $@)
	g++ -c $< $(CXXFLAGS_DEBUG) -o $@

ifneq ($(wildcard $(CXXTEST_HOME)),)

bin/test: $(TEST_OBJECT_FILES_RELEASE) $(OBJECT_FILES_RELEASE)
	mkdir -p $(dir $@)
	g++  $^ $(LDFLAGS_RELEASE) -o $@

bin/test_d: $(TEST_OBJECT_FILES_DEBUG) $(OBJECT_FILES_DEBUG)
	mkdir -p $(dir $@)
	g++  $^ $(LDFLAGS_DEBUG) -o $@

build/test/%.o: build/test/%.cpp $(HEADER_FILES) Makefile
	mkdir -p $(dir $@)
	g++ -c $< $(CXXFLAGS_RELEASE) -o $@

build/test/%_d.o: build/test/%.cpp $(HEADER_FILES) Makefile
	mkdir -p $(dir $@)
	g++ -c $< $(CXXFLAGS_DEBUG) -o $@

.PRECIOUS: build/test/%.cxxtest.cpp
build/test/%.cxxtest.cpp: test/%.cxxtest.hpp
	mkdir -p $(dir $@)
	$(CXXTEST_GEN) --part --error-printer $< -o $@

build/test/runner.cpp:
	mkdir -p $(dir $@)
	$(CXXTEST_GEN) --root --error-printer -o $@

else

bin/test:
	@echo "Cannot find CxxTest to build tests."
	false

endif

clean:
	rm -rf bin/ build/ doc/ *.gcda *.gcno *.gcov gmon.out
