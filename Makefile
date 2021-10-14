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
CFLAGS	?= -O2
CFLAGS	+= -g -fsigned-char -D_GNU_SOURCE
LDFLAGS =
INSTALL	= /usr/bin/install -c
SOUND_INC = -I.

PROGS	= clock mousemode nvsetenv trackpad backlight nvsetvol \
	fblevel fnset lsprop autoboot bootsched
SCRIPTS	= macos

GENERATED_MAN = clock.8 macos.8 nvsetenv.8 sndvolmix.8 fdeject.1 \
	mousemode.8 nvvideo.8 vmode.8 autoboot.8


# DEPENDENCIES:
all:	$(PROGS) $(SCRIPTS)

autoboot:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

bootsched:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

clock:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

fdeject:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

mousemode:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

trackpad:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

backlight:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

fblevel:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

fnset:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

lsprop:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

nvsetenv: nvsetenv.c nwnvsetenv.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c nwnvsetenv.c

nvsetvol:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

nvvideo:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

sndvolmix:
	$(CC) $(CFLAGS) $(SOUND_INC) $(LDFLAGS) -o $@ $@.c

vmode:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

all-man: autoboot.8 clock.8 mousemode.8 nvsetenv.8 sndvolmix.8 vmode.8 macos.8 nvvideo.8 fdeject.8 fblevel.8

autoboot.8: autoboot.sgml
	$(SGMLMAN) $< && mv -f autoboot.man $@
bootsched.8: bootsched.sgml
	$(SGMLMAN) $< && mv -f bootsched.man $@

clock.8: clock.sgml
	$(SGMLMAN) $< && mv -f clock.man $@
mousemode.8: mousemode.sgml
	$(SGMLMAN) $< && mv -f mousemode.man $@
nvsetenv.8: nvsetenv.sgml
	$(SGMLMAN) $< && mv -f nvsetenv.man $@

sndvolmix.8: sndvolmix.sgml
	$(SGMLMAN) $< && mv -f sndvolmix.man $@
vmode.8: vmode.sgml
	$(SGMLMAN) $< && mv -f vmode.man $@
macos.8: macos.sgml
	$(SGMLMAN) $< && mv -f macos.man $@
nvvideo.8: nvvideo.sgml
	$(SGMLMAN) $< && mv -f nvvideo.man $@
fdeject.8: fdeject.sgml
	$(SGMLMAN) $< && mv -f fdeject.man $@

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
	$(INSTALL) -m 755   mousemode sndvolmix vmode nvsetvol $(DESTDIR)$(usbindir)
	$(INSTALL) -m 755   fdeject lsprop $(DESTDIR)/$(ubindir)
	$(INSTALL) -m 755   $(SCRIPTS) $(DESTDIR)$(sbindir)

	$(INSTALL) -m 644   fdeject.man	$(DESTDIR)$(man1dir)/fdeject.1
	$(INSTALL) -m 644   clock.man	$(DESTDIR)$(man8dir)/clock.8
	$(INSTALL) -m 644   autoboot.man	$(DESTDIR)$(man8dir)/autoboot.8
	$(INSTALL) -m 644   mousemode.man	$(DESTDIR)$(man8dir)/mousemode.8
	$(INSTALL) -m 644   nvsetenv.man	$(DESTDIR)$(man8dir)/nvsetenv.8
	$(INSTALL) -m 644   nvvideo.man	$(DESTDIR)$(man8dir)/nvvideo.8
	$(INSTALL) -m 644   sndvolmix.man	$(DESTDIR)$(man8dir)/sndvolmix.8
	$(INSTALL) -m 644   vmode.man	$(DESTDIR)$(man8dir)/vmode.8
	$(INSTALL) -m 644   macos.man	$(DESTDIR)$(man8dir)/macos.8
	$(INSTALL) -m 644   trackpad.8	$(DESTDIR)$(man8dir)/trackpad.8
	$(INSTALL) -m 644   nvsetvol.8	$(DESTDIR)$(man8dir)/nvsetvol.8

cleanobjs:
	$(RM) *.o *.bak *~ *.man $(GENERATED_MAN)

clean:	cleanobjs
	$(RM) $(PROGS)
