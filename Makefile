# Output build directoy.
BUILD = build

# Look for clang++ on the system.
#CLANGPP_PATH = $(shell which clang++)
#ifeq ($(shell if [ -e $(CLANGPP_PATH) ]; then echo "t"; fi), t)
#  CXX_OPTION = CXX=$(CLANGPP_PATH)
#endif


.PHONY: default
default: ck
	make -C $(BUILD)

# Create build directory.
.PHONY: builddir
builddir:
	@echo "Creating '$(BUILD)' directory..."
	mkdir -p $(BUILD)

# Set up symlinks to necessary files.
.PHONY: symlinks
symlinks: builddir
	@echo "Making symlinks..."
	ln -sf ../Makefile.am $(BUILD)
	ln -sf ../configure.ac $(BUILD)
	ln -sf ../m4 $(BUILD)
	ln -sf ../src $(BUILD)
	ln -sf ../tests $(BUILD)

# Reconfigure autotools stuff.
.PHONY: autoreconf
autoreconf: symlinks
	@echo "Running autoreconf..."
	cd $(BUILD) && autoreconf -i

# Run configure script.
.PHONY: configure
configure: autoreconf
ifdef CXX_OPTION
	@echo "Using clang++: $(CLANGPP_PATH)"
endif
	@echo "Running ./configure $(OPTS) ..."
	cd $(BUILD) && ./configure $(OPTS) $(CXX_OPTION)

# Build Concurrency Kit.
.PHONY: ck
ck:
	@echo "Building Concurrency Kit..."
	cd $(BUILD)/src/ck && CC=gcc ./configure --profile=x86 && make
	rm -f $(BUILD)/src/ck/src/libck.so  # avoid linking with shared library

.PHONY: all
all: builddir symlinks autoreconf configure ck
	make -C $(BUILD) all

# Package up the project.
.PHONY: dist
dist: configure
	make -C $(BUILD) dist

# Run tests.
.PHONY: check
check:
	make -C $(BUILD) check

# Clean the build.
.PHONY: clean
clean:
	make -C $(BUILD)/src/ck clean
	make -C $(BUILD) clean

# Hose everything.
.PHONY: deep-clean
deep-clean:
	rm -rf $(BUILD)