CC      = gcc
INCDIR  = ../include
CFLAGS  = -Zomf -Zexe -I$(INCDIR)
OBJS    = os2font.o gpifont.o

os2font :	$(OBJS)
		gcc $(CFLAGS) $(OBJS) -o $@

clean :
		rm -f $(OBJS)
		rm -f os2font os2font.exe
