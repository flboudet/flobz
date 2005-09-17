#include ../root_dir
include ../config

SRCS=PuyoServerMain.cpp

OBJECTS=${SRCS:%.cpp=%.o}

CFLAGS:=$(CFLAGS) -I../iosfc

all: prelude bin

prelude:
	@echo "PLATFORM: $(PLATFORM)"
	@echo "PLATFORMTYPE: $(PLATFORMTYPE)"

bin: ${OBJECTS}
	$(CXX) -o fpserver ${OBJECTS} -L../iosfc -liosfc

DEPDIR = .deps
df = $(DEPDIR)/$(*F)

MAKEDEPEND = $(CXX) -MM $(CFLAGS_NOPCH) -o $(df).d $<

%.o:%.cpp
	@mkdir -p $(DEPDIR); \
	$(MAKEDEPEND);  \
	cp $(df).d $(df).P;  \
	cat $(df).d | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' >> $(df).P; \
	rm -f $(df).d
	@echo "[$@]" && $(CXX) $(CFLAGS) -c $< # 2>> WARNINGS || (cat WARNINGS && false)

clean:
	rm -f *.o *~

.PHONY: all object clean

-include $(SRCS:%.cpp=$(DEPDIR)/%.P)
# DO NOT DELETE
