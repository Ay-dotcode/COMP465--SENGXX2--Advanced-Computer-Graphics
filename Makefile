# Compiler and flags
CXX = g++
CXXFLAGS = -w -O2       # -w disables all warnings
LDFLAGS = $(shell pkg-config --libs libpng)

.PHONY: clean

# Pattern rule: build and run any .cpp file
%: %.cpp
	@echo "🔧 Compiling $<..."
	@if $(CXX) $(CXXFLAGS) "$<" -o "$@" $(LDFLAGS); then \
		echo "✅ Compilation successful!"; \
		echo "🚀 Running $@..."; \
		./"$@"; \
	else \
		echo "❌ Compilation failed."; \
		exit 1; \
	fi

# Clean rule
clean:
	@rm -f *.o *.out *
