# IBM C/C++ (VisualAge) Makefile for CPMAP
#

DEBUG  = 1

INCDIR = ..\include

CC     = icc.exe
RC     = rc.exe
LINK   = ilink.exe
CFLAGS = /Gm /Ss /Sp1 /Q /Wpar /Wuse /I$(INCDIR)
RFLAGS = -n -i $(INCDIR)
LFLAGS = /PMTYPE:PM /NOLOGO /MAP
NAME   = compfont
OBJS   = $(NAME).obj abr.obj combined.obj unifont.obj
MRI    = $(NAME)
LIBS   =

!ifdef DEBUG
    CFLAGS   = $(CFLAGS) /Ti /Tm
    LFLAGS   = $(LFLAGS) /DEBUG /STACK:0x10000
!endif


$(NAME).exe : $(OBJS) $(NAME).res
                $(LINK) $(LFLAGS) $(OBJS) $(LIBS)
                $(RC) $(RFLAGS) $(MRI).res $@

$(NAME).res : $(NAME).rc dialog.dlg program.ico {$(INCDIR)}ids.h
                $(RC) $(RFLAGS) -r $(NAME).rc $@

$(NAME).obj : {$(INCDIR)}$(NAME).h {$(INCDIR)}ids.h {$(INCDIR)}cmbfont.h {$(INCDIR)}unifont.h {$(INCDIR)}gpifont.h

clean       :
              -del $(OBJS) $(NAME).res $(NAME).exe $(NAME).map 2>NUL


