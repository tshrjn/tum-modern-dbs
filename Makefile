CC = clang++
CXXFLAGS = -std=c++11 -O3 -Wall -Isrc
CHECKDIR=@mkdir -p $(dir $@)

BUILDEXE=$(CC) -o$@ $(CXXFLAGS) $^

-include bin/*.d bin/*/*.d 

include src/LocalMakefile
include test/LocalMakefile

all: bin/bufferTest bin/parserTest bin/schemaTest bin/slottedPageTest bin/slottedTest

clean:
	find bin -name '*.d' -delete -o -name '*.o' -delete -o '(' -perm -u=x '!' -type d ')' -delete
