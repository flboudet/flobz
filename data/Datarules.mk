.PHONY: all $(FILES) $(DIRECTORIES)

install: $(FILES) $(DIRECTORIES)

$(FILES):
	@echo "Installing file $@"
	@cp $@ ${INSTALL_DATADIR}/$@

$(DIRECTORIES):
	@echo "Installing directory $@"
	@mkdir -p ${INSTALL_DATADIR}/$@
	@+make -C $@ install INSTALL_DATADIR="${INSTALL_DATADIR}/$@" SOURCE_DATADIR="$(SOURCE_DATADIR)"

