targets = tcp
src_fies = $(addsuffix _example, $(targets))
clean_src_files = $(addsuffix _, $(targets))

.PHONY: test

test:
	$(MAKE) -s $(targets)
	$(MAKE) -s clean

$(targets): $(src_fies)
	@echo $(shell ./test_$@.sh)

$(src_fies):
	@echo Making $@ ...
	$(MAKE) -C ../$@
	@echo Finish making $@


clean: $(clean_src_files)

$(clean_src_files):
	cd ../$@example && $(MAKE) clean;
	rm -f *.dat
