#
# Makefile      Makefile for Powermac/Linux specific programs
#               Targets:   all		compiles everything
#                          install	installs the binaries
#			   installdevs	creates necessary devices
#                          clean	cleans up
#
# Version:      @(#)Makefile  1.01  06-May-1997  Richard van Hees
#

prefix =
exec_prefix = ${prefix}
usr_prefix  = ${prefix}/usr
bindir	 = $(exec_prefix)/bin
devsdir  = ${prefix}/dev
ubindir  = $(usr_prefix)/bin
sbindir  = $(exec_prefix)/sbin
usbindir = $(usr_prefix)/sbin
mandir	 = $(usr_prefix)/man
man1dir  = $(usr_prefix)/man/man1
man8dir  = $(usr_prefix)/man/man8

SGMLMAN	= sgml2txt -man
CC	= gcc -Wall -Wstrict-prototypes
CFLAGS	= -O2 -fsigned-char 
LDFLAGS = -s
INSTALL	= /usr/bin/install -c
SOUND_INC = -I.

PROGS	= clock fdeject mousemode nvsetenv nvvideo sndvolmix vmode
SCRIPTS	= macos

# DEPENDENCIES:
all:	$(PROGS) $(SCRIPTS)

clock:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

fdeject:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

mousemode:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

nvsetenv:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

nvvideo:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

sndvolmix:
	$(CC) $(CFLAGS) $(SOUND_INC) $(LDFLAGS) -o $@ $@.c

vmode:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

man:
	$(SGMLMAN) clock.sgml
	$(SGMLMAN) fdeject.sgml
	$(SGMLMAN) mousemode.sgml
	$(SGMLMAN) nvsetenv.sgml
	$(SGMLMAN) nvvideo.sgml
	$(SGMLMAN) sndvolmix.sgml
	$(SGMLMAN) vmode.sgml
	$(SGMLMAN) macos.sgml

installdirs:
	./mkinstalldirs $(DESTDIR)$(sbindir) $(DESTDIR)$(ubindir) \
			$(DESTDIR)$(man1dir) $(DESTDIR)$(man8dir) \
			$(DESTDIR)$(usbindir) $(DESTDIR)$(devsdir)

installdevs:
	@if [ ! -c $(DESTDIR)$(devsdir)/adb ]; then \
		echo "Creating $(DESTDIR)$(devsdir)/adb ..."; \
		mknod $(DESTDIR)$(devsdir)/adb c 56 0; \
		chmod 600 $(DESTDIR)$(devsdir)/adb; \
	fi
	@if [ ! -c $(DESTDIR)$(devsdir)/nvram ]; then \
		echo "Creating $(DESTDIR)$(devsdir)/nvram ..."; \
		mknod $(DESTDIR)$(devsdir)/nvram c 10 144; \
		chmod 644 $(DESTDIR)$(devsdir)/nvram; \
	fi
	@if [ ! -c $(DESTDIR)$(devsdir)/mixer ]; then \
		echo "Creating $(DESTDIR)$(devsdir)/mixer ..."; \
		mknod $(DESTDIR)$(devsdir)/mixer c 14 0; \
		chmod 644 $(DESTDIR)$(devsdir)/mixer; \
	fi
	@if [ ! -c $(DESTDIR)$(devsdir)/dsp ]; then \
		echo "Creating $(DESTDIR)$(devsdir)/dsp ..."; \
		mknod $(DESTDIR)$(devsdir)/dsp c 14 3; \
		chmod 644 $(DESTDIR)$(devsdir)/dsp; \
	fi
	@if [ ! -c $(DESTDIR)$(devsdir)/audio ]; then \
		echo "Creating $(DESTDIR)$(devsdir)/audio ..."; \
		mknod $(DESTDIR)$(devsdir)/audio c 14 4; \
		chmod 644 $(DESTDIR)$(devsdir)/audio; \
	fi
	@if [ ! -c $(DESTDIR)$(devsdir)/sndstat ]; then \
		echo "Creating $(DESTDIR)$(devsdir)/sndstat ..."; \
		mknod $(DESTDIR)$(devsdir)/sndstat c 14 6; \
		chmod 600 $(DESTDIR)$(devsdir)/sndstat; \
	fi

install:	all man installdirs
	$(INSTALL) -m 4511  clock $(DESTDIR)$(sbindir) 
	$(INSTALL) -m 755   nvsetenv nvvideo $(DESTDIR)$(sbindir)
	$(INSTALL) -m 755   mousemode sndvolmix vmode $(DESTDIR)$(usbindir)
	$(INSTALL) -m 755   fdeject $(DESTDIR)/$(ubindir)
	$(INSTALL) -m 755   $(SCRIPTS) $(DESTDIR)$(sbindir)

	$(INSTALL) -m 644   fdeject.man	$(DESTDIR)$(man1dir)/fdeject.1
	$(INSTALL) -m 644   clock.man	$(DESTDIR)$(man8dir)/clock.8
	$(INSTALL) -m 644   mousemode.man	$(DESTDIR)$(man8dir)/mousemode.8
	$(INSTALL) -m 644   nvsetenv.man	$(DESTDIR)$(man8dir)/nvsetenv.8
	$(INSTALL) -m 644   nvvideo.man	$(DESTDIR)$(man8dir)/nvvideo.8
	$(INSTALL) -m 644   sndvolmix.man	$(DESTDIR)$(man8dir)/sndvolmix.8
	$(INSTALL) -m 644   vmode.man	$(DESTDIR)$(man8dir)/vmode.8
	$(INSTALL) -m 644   macos.man	$(DESTDIR)$(man8dir)/macos.8

cleanobjs:
	$(RM) *.o *.bak *~ *.man *.1 *.8

clean:	cleanobjs
	$(RM) $(PROGS)
