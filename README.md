![Shikoba!](http://queatz.com/static/Shikoba.png)

Shikoba is a C++ text library for OpenGL 3 and above using the FreeType 2 library.

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

// Set the face to be used
fontlib->setFace(fontface);

// Set the face size to be used
fontlib->setSize(16);

// Fetch a glyph using utf-8 index
Shikoba::Glyph * g;
g = fontlib->glyph('A');

// Do something with the glyph, like store it in a vbo
// g.vertices
// g.texcoords
// Both are a Shikoba::Box, with .x1, .y1, .x2, .y2
// 1 = lower left, 2 = upper right
// And bind the texture using the OpenGL texture id of the library:
Gluint t = fontlib->texture();

// If you wanted to write out a string, advance your cursor with
float next_pen_x = fontlib->advance(previous_char, next_char);
```
