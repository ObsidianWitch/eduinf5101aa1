include $(PVM_ROOT)/conf/$(PVM_ARCH).def

CFLAGS	=	-I$(PVM_ROOT)/include $(ARCHCFLAGS)

LIBS	=	-lpvm3 -lgpvm3 $(ARCHLIB) -lm
LFLAGS	=	-L$(PVM_ROOT)/lib/$(PVM_ARCH)
LDFLAGS	=	$(LFLAGS) $(LIBS)

.c:
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	mkdir -p $(HOME)/pvm3/bin/$(PVM_ARCH)
	mv $@ $(HOME)/pvm3/bin/$(PVM_ARCH)
