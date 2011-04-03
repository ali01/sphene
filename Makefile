# Output build directoy.
BUILD = build

# Look for clang++ on the system.
#CLANGPP_PATH = $(shell which clang++)
#ifeq ($(shell if [ -e $(CLANGPP_PATH) ]; then echo "t"; fi), t)
#  CXX_OPTION = CXX=$(CLANGPP_PATH)
#endif


.PHONY: default
default:
	make -C $(BUILD)

# Create build directory.
.PHONY: builddir
builddir:
	@echo "Creating '$(BUILD)' directory..."
	mkdir -p $(BUILD)

# Set up symlinks to necessary files.
.PHONY: symlinks
symlinks:
	@echo "Making symlinks..."
	ln -sf ../Makefile.am $(BUILD)
	ln -sf ../configure.ac $(BUILD)
	ln -sf ../m4 $(BUILD)
	ln -sf ../src $(BUILD)
	ln -sf ../tests $(BUILD)
	ln -sf ../gtest-1.6.0 $(BUILD)

# Reconfigure autotools stuff.
.PHONY: autoreconf
autoreconf:
	@echo "Running autoreconf..."
	cd $(BUILD) && autoreconf -i

# Run configure script.
.PHONY: configure
configure:
ifdef CXX_OPTION
	@echo "Using clang++: $(CLANGPP_PATH)"
endif
	@echo "Running configure..."
	cd $(BUILD) && ./configure $(CXX_OPTION)

.PHONY: all
all: builddir symlinks autoreconf configure
	make -C $(BUILD) all

# Run tests.
.PHONY: check
check:
	make -C $(BUILD) check

# Clean the build.
.PHONY: clean
clean:
	make -C $(BUILD) clean

# Hose everything.
.PHONY: deep-clean
deep-clean:
	rm -rf $(BUILD)