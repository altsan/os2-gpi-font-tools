ifndef $(OS)
  OS := $(shell uname)
endif

CC        = gcc
OBJS      = os2font.o gpifont.o
INCDIR    = ../include
CFLAGS    = -I$(INCDIR)

ifeq ($(OS),OS/2)
  CFLAGS += -Zomf
  EEXT    = .exe
endif
ifeq ($(OS),Windows_NT)
  EEXT    = .exe
  RM      = del
else
  RM      = rm -f
endif

ifeq ($(DEBUG),1)
  CFLAGS  += -g
endif


os2font$(EEXT):	$(OBJS)
		gcc $(CFLAGS) $(OBJS) -o $@

clean:
		$(RM) $(OBJS) os2font$(EEXT)
