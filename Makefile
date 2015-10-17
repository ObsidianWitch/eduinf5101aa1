include $(PVM_ROOT)/conf/$(PVM_ARCH).def

CFLAGS	=	-I$(PVM_ROOT)/include $(ARCHCFLAGS) -Wall -std=c99

LIBS	=	-lpvm3 -lgpvm3 $(ARCHLIB) -lm
LFLAGS	=	-L$(PVM_ROOT)/lib/$(PVM_ARCH)
LDFLAGS	=	$(LFLAGS) $(LIBS)

gaussp: gaussp.c

debug: CFLAGS += -DDEBUG
debug: gaussp

.c:
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	mkdir -p $(HOME)/pvm3/bin/$(PVM_ARCH)
	mv $@ $(HOME)/pvm3/bin/$(PVM_ARCH)
