# TUM: Modern DBs

## Assignment 01

Run with:
```bash
$ make sort
$ ./bin/sort <inputFile> <outputFile> <memoryBufferInMiB>
```

## Assignment 02

Run with:
```bash
$ make buffer
$ ./bin/buffertest <pagesOnDisk> <pagesInRAM> <threads>
```

## Assignment 03

Run & test with:
```bash
$ make all
$ ./bin/schemaTest test/test.sql
$ ./bin/schemaTest test/testLong.sql
$ ./bin/pidTest
$ ./bin/tidTest
$ ./bin/slottedPageTest
$ ./bin/slottedTest 16384
```

## Assignment 04

Run with:
```bash
$ make all
$ ./bin/btreeTest <numberOfElements>
```
for example:
```bash
$ ./bin/btreeTest 100000
```

## Assignment 04

Run with:
```bash
$ make all
$ ./bin/operatorTest
```