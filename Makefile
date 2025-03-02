# Membrane Makefile with file watching and hot reload

# Project directories
PROJECT_DIR := $(shell pwd)
REACT_DIR := $(PROJECT_DIR)/src-react
BUILD_DIR := $(PROJECT_DIR)/build
DEV_BUILD_DIR := $(PROJECT_DIR)/build-dev
RES_DIR := $(PROJECT_DIR)/res
PID_DIR := $(DEV_BUILD_DIR)/.pids

# Commands
CMAKE := cmake
MAKE := make
NPM := bun
FSWATCH := fswatch

# Default target
all: production

# Development mode with file watching
dev: setup-dev
	@echo "Starting development mode with file watching..."
	@mkdir -p $(DEV_BUILD_DIR)
	@mkdir -p $(PID_DIR)
	@cd $(DEV_BUILD_DIR) && $(CMAKE) -DDEV_MODE=ON .. && $(MAKE)
	@cd $(REACT_DIR) && $(NPM) run dev & echo $$! > $(PID_DIR)/vite.pid
	@sleep 2 # Give Vite server time to start
	@$(MAKE) watch
	@echo "Starting application..."
	@( \
		trap 'make stop' EXIT INT TERM; \
		cd $(DEV_BUILD_DIR) && ./Membrane; \
		make stop; \
	)

# Production build
production:
	@echo "Building production version..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. && $(MAKE)

# Setup development environment
setup-dev:
	@echo "Setting up development environment..."
	@command -v $(FSWATCH) >/dev/null 2>&1 || { echo "Please install fswatch first: brew install fswatch or apt-get install fswatch"; exit 1; }
	@cd $(REACT_DIR) && $(NPM) install
	@# Kill any lingering processes
	@$(MAKE) stop >/dev/null 2>&1 || true

# Watch for C++ file changes (run in background)
watch:
	@echo "Watching for C++ file changes..."
	@$(FSWATCH) -o $(PROJECT_DIR)/lib $(PROJECT_DIR)/src | xargs -n1 -I{} $(MAKE) rebuild & echo $$! > $(PID_DIR)/watcher.pid

# Rebuild C++ code when changes detected
rebuild:
	@echo "Rebuilding C++ code..."
	@cd $(DEV_BUILD_DIR) && $(MAKE)
	@echo "Rebuild complete!"

# Clean up
clean: stop
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(DEV_BUILD_DIR)

# Stop development servers
stop:
	@echo "Stopping development servers..."
	@if [ -d $(PID_DIR) ]; then \
		for pid_file in $(PID_DIR)/*.pid; do \
			if [ -f $$pid_file ]; then \
				pid=$$(cat $$pid_file); \
				echo "Stopping process $$pid"; \
				kill -15 $$pid 2>/dev/null || kill -9 $$pid 2>/dev/null || true; \
				rm $$pid_file; \
			fi; \
		done; \
	fi
	@# Find and kill any lingering Node.js/Vite/Bun processes on port 5173
	@pid=$$(lsof -ti:5173 2>/dev/null); \
	if [ "$$pid" != "" ]; then \
		echo "Killing process on port 5173: $$pid"; \
		kill -15 $$pid 2>/dev/null || kill -9 $$pid 2>/dev/null || true; \
	fi

.PHONY: all dev production setup-dev watch rebuild clean stop