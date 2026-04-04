CC = gcc # Compiler
MAKE = make
CFLAGS = -g -Wno-unused-but-set-variable -Wno-parentheses -Wno-misleading-indentation -Wno-deprecated-declarations -Wno-pointer-sign -Wall -pthread -I/usr/include/gtk-3.0 -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/include/gtk-3.0 -I/usr/include/gio-unix-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/fribidi -I/usr/include/harfbuzz -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/x86_64-linux-gnu -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml # Compile Flags
LDFLAGS = -lm -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lxml2 # Linker FLags
NAME = papergen

OBJECT_FILES = gui.o question_database.o load_pdf.o structures.o system_commands.o # All the object files I want to link

all: $(NAME) clean # Build tasks

$(NAME): main.c $(OBJECT_FILES) # Compile executable
	$(CC) -o $(NAME) -no-pie $(CFLAGS) main.c $(OBJECT_FILES) $(LDFLAGS) # Compile and link

system_commands.o: system_commands.c
	$(CC) -c $(CFLAGS) -o system_commands.o system_commands.c

load_pdf.o: load_pdf.c
	$(CC) -c $(CFLAGS) -o load_pdf.o load_pdf.c

gui.o: gui.c
	$(CC) -c $(CFLAGS) -o gui.o gui.c

question_database.o: question_database.c
	$(CC) -c $(CFLAGS) -o question_database.o question_database.c

structures.o: structures.c
	$(CC) -c $(CFLAGS) -o structures.o structures.c

.PHONY: clean # Not a real target
clean: # Cleanup tasks
	rm $(OBJECT_FILES) # Delete all this stuff
