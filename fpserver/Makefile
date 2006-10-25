#include ../root_dir
include ../config

SRCS=PuyoServerMain.cpp PuyoServerIgpResponder.cpp PuyoServerIgpNatTraversal.cpp

OBJECTS=${SRCS:%.cpp=%.o}

CFLAGS:=$(CFLAGS) -I../iosfc -I..


all: prelude bin

prelude:
	@echo "PLATFORM: $(PLATFORM)"
	@echo "PLATFORMTYPE: $(PLATFORMTYPE)"

bin: ${OBJECTS}
	$(CXX) -o fpserver ${OBJECTS} -L../iosfc -liosfc

DEPDIR = .deps
df = $(DEPDIR)/$(*F)

MAKEDEPEND = $(CXX) -MM $(CFLAGS) -o $(df).d $<

%.o:%.cpp
	@mkdir -p $(DEPDIR); \
	$(MAKEDEPEND);  \
	cp $(df).d $(df).P;  \
	cat $(df).d | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' >> $(df).P; \
	rm -f $(df).d
	@echo "[$@]" && $(CXX) $(CFLAGS) -c $< # 2>> WARNINGS || (cat WARNINGS && false)

test_igpudppuncherclient: test_igpudppuncherclient.cpp ../PuyoNatTraversal.cpp
	$(CXX) -o test_igpudppuncherclient -I.. -I../iosfc test_igpudppuncherclient.cpp ../PuyoNatTraversal.cpp -L../iosfc -liosfc

clean:
	rm -f *.o *~

.PHONY: all object clean

-include $(SRCS:%.cpp=$(DEPDIR)/%.P)
# DO NOT DELETE
