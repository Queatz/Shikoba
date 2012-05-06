![Shikoba!](http://queatz.com/static/Shikoba.png)

Shikoba is a C++ text library for OpenGL 3 and above using the FreeType 2 library.

It fills a single GL_TEXTURE_RECTANGLE with a GL_R8 charmap and returns vertex and texture coordinate information for each requested glyph.  The texture ID and glyph information never change and are valid until the destruction of the Library object.

Depends
------

FreeType 2  
http://freetype.org/

Quickstart
---------

C++:

```c++
// Create a library
Shikoba::Library * fontlib = new Shikoba::Library();

// Create a face in the library
Shikoba::Face * fontface = new Shikoba::Face(fontlib, "fontfile.ttf");

// Set the face size to be used
fontface->size(16);

// Fetch a glyph using utf-8 index
Shikoba::Glyph * g;
g = fontface->glyph('A');

// Do something with the glyph, like store it in a vbo
// g.vertices
// g.texcoords
// Both are GLfloat[4] containing the lower-left corner in the first two values and the upper right corner in the last two values
// And bind the texture using the OpenGL texture id of the library:
Gluint t = fontlib->texture();

// Check for any errors
const char * err = fontlib->getErrorString();

// If you wanted to write out a string, advance your cursor with
float next_pen_x = fontface->advance(previous_char, next_char);
```

Quickref
-------

### Library

* Takes no arguments.
* `.texture()` returns the OpenGL texture ID.
* `.getErrorString()` returns the string of the latest error.

### Face

* Takes two or three arguments: `(Shikoba::Library * fontlib, const char * filename, unsigned int size = 0)`
* `.size(unsigned int)` sets the face size.
* `.glyph(uint32_t)` returns a `Shikoba::Glyph`.
* `.advance(uint32_t previous_char, uint32_t next_char)` returns the advance between two glyphs.
* `.height()` returns the line height.
* `.ascender()` returns the ascender value.
* `.descender()` returns the descender value.

### Glyph

* These are returned by `Shikoba::Face::glyph()`.
* `.vertices` is a 4-element GLfloat array of vertex positions whereas the first two are the lower-left corner, and the last two are the upper-right corner.
* `.texcoords` is a 4-element GLfloat array of texture coordinates whereas the first two are the lower-left corner, and the last two are the upper-right corner.
