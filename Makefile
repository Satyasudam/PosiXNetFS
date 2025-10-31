SUBDIRS = server client tests

.PHONY: all clean

all:
	@for dir in $(SUBDIRS); do \
		echo "Building in $$dir..."; \
		$(MAKE) -s -C $$dir || exit 1; \
	done
	@echo "Build complete."

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -s -C $$dir clean; \
	done
	@echo "Clean complete."

