
CC = clang++
CXXFLAGS = -std=c++11 -O3 -Wall -Isrc
CHECKDIR=@mkdir -p $(dir $@)

BUILDEXE=$(CC) -o$@ $(CXXFLAGS) $^

-include bin/*.d bin/*/*.d 

all: bin/bufferTest bin/parserTest bin/schemaTest bin/slottedTest

bin/%.o: src/%.cpp 
	$(CHECKDIR)
	$(CC) -o$@ -c $(CXXFLAGS) $<
obj_segment:=bin/SchemaSegment.o bin/Segment.o bin/SPSegment.o

bin/buffer/%.o: src/buffer/%.cpp 
	$(CHECKDIR)
	$(CC) -o$@ -c $(CXXFLAGS) $<
obj_buffer:=bin/buffer/BufferFrame.o bin/buffer/BufferManager.o

obj_schema:=bin/schema/Parser.o bin/schema/Schema.o bin/schema/Types.o
bin/schema/%.o: src/schema/%.cpp 
	$(CHECKDIR)
	$(CC) -o$@ -c $(CXXFLAGS) $<

bin/pages/%.o: src/pages/%.cpp 
	$(CHECKDIR)
	$(CC) -o$@ -c $(CXXFLAGS) $<
obj_pages:=bin/pages/Record.o bin/pages/SlottedPage.o bin/pages/TID.o

bin/sort/%.o: src/sort/%.cpp 
	$(CHECKDIR)
	$(CC) -o$@ -c $(CXXFLAGS) $<
obj_sort:=bin/sort/ExternalSort.o


bin/test/%.o: test/%.cpp 
	$(CHECKDIR)
	$(CC) -o$@ -c $(CXXFLAGS) $<
bin/bufferTest: bin/test/bufferTest.o $(obj_buffer)
	$(BUILDEXE)
bin/parserTest: bin/test/parserTest.o $(obj_schema)
	$(BUILDEXE)
bin/schemaTest: bin/test/schemaTest.o $(obj_buffer) $(obj_schema) $(obj_segment)
	$(BUILDEXE)
bin/slottedTest: bin/test/slottedTest.o $(obj_buffer) $(obj_schema) $(obj_pages) $(obj_segment)
	$(BUILDEXE)

clean:
	find bin -name '*.d' -delete -o -name '*.o' -delete -o '(' -perm -u=x '!' -type d ')' -delete
