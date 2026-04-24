CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRCDIR   = src
OUTDIR   = output

SOURCES  = $(SRCDIR)/main.cpp         \
           $(SRCDIR)/grammar.cpp      \
           $(SRCDIR)/items.cpp        \
           $(SRCDIR)/parsing_table.cpp\
           $(SRCDIR)/slr_parser.cpp   \
           $(SRCDIR)/lr1_parser.cpp   \
           $(SRCDIR)/tree.cpp

TARGET   = parser

.PHONY: all clean run-slr run-lr1 run-both run-compare dirs

all: dirs $(TARGET)

dirs:
	mkdir -p $(OUTDIR)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

# ---- convenience targets ----
run-slr: all
	./$(TARGET) --slr input/grammar2.txt input/input_valid.txt

run-lr1: all
	./$(TARGET) --lr1 input/grammar2.txt input/input_valid.txt

run-both: all
	./$(TARGET) --both input/grammar2.txt input/input_valid.txt

run-compare: all
	./$(TARGET) --compare input/grammar3.txt

run-conflict: all
	./$(TARGET) --both input/grammar3.txt input/input_valid.txt

run-invalid: all
	./$(TARGET) --slr input/grammar2.txt input/input_invalid.txt

valgrind: all
	valgrind --leak-check=full --error-exitcode=1 \
	    ./$(TARGET) --both input/grammar2.txt input/input_valid.txt

clean:
	rm -f $(TARGET)
	rm -f $(OUTDIR)/*.txt
