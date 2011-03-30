# Output build directoy.
BUILD = build

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

# Reconfigure autotools stuff.
.PHONY: autoreconf
autoreconf:
	@echo "Running autoreconf..."
	cd $(BUILD) && autoreconf -i

# Run configure script.
.PHONY: configure
configure:
	@echo "Running configure..."
	cd $(BUILD) && ./configure

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