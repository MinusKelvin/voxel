SOURCEDIR := src
BINDIR := bin
SOURCES := $(shell find $(SOURCEDIR) -name '*.c')
OBJECTS := $(shell find src/ -name '*.c' | sed -e 's/src/$(BINDIR)/' -e 's/.c$$/.o/')
DEPHEADERS := $(shell find src/ -name '*.h')

$(BINDIR)/%.o: $(SOURCEDIR)/%.c $(DEPHEADERS)
	gcc -Wall -c $$(pkg-config --cflags glfw3) -o $@ $< $$(pkg-config --static --libs glfw3) -lGLEW -lGL

voxel: $(OBJECTS)
	gcc -Wall $$(pkg-config --cflags glfw3) -o $@ $(OBJECTS) $$(pkg-config --static --libs glfw3) -lGLEW -lGL

clean:
	rm -f $(shell find bin -name '*.o') voxel
