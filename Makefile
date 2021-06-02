TARGETS	:= spmp_isp spmp_playground spmp_iorw loaders

.PHONY: all $(TARGETS) clean
all: $(TARGETS)

$(TARGETS): .FORCE
	@$(MAKE) --no-print-directory -C $@ all

clean:
	@set -e; for tgt in $(TARGETS); do \
		$(MAKE) --no-print-directory -C $$tgt clean; \
	done

.FORCE:
