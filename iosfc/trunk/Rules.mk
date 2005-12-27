DEPDIR = .deps
df = $(DEPDIR)/$(*F)

MAKEDEPEND = $(CXX) -MM $(CXXFLAGS) -o $(df).d $<
# MAKEDEPEND = touch $*.d && makedepend $(CXXFLAGS) -f $*.d $<

%.o:%.cpp
	@mkdir -p $(DEPDIR); \
	$(MAKEDEPEND);  \
	cp $(df).d $(df).P;  \
	cat $(df).d | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' >> $(df).P; \
	rm -f $(df).d
	@echo "[$@]" && $(CXX) $(CXXFLAGS) -c $< # 2>> WARNINGS || (cat WARNINGS && false)

clean: clean-recursive
	rm -f *.o *~ *.a
	rm -rf $(DEPDIR)

clean-recursive:
	@+for subdir in $(SUBDIRECTORIES); do \
	make -C $$subdir clean; \
	done

.PHONY: clean clean-recursive

-include $(SRCS:%.cpp=$(DEPDIR)/%.P)
# DO NOT DELETE
