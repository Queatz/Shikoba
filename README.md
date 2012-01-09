Shikoba is a C++ text library for OpenGL 3 and above using the FreeType 2 library.

Depends
------

FreeType 2
http://freetype.org/

UTF8-CPP
http://utfcpp.sourceforge.net/

Quickstart
---------

C++:

	// Create a library
	Shikoba::Library * fontlib = new Shikoba::Library();

	// Create a face in the library
	Shikoba::Font * fontface = new Shikoba::Face(fontlib, "fontfile.ttf");

	// Create a sized font out of a face
	// id_of_gl_texture is a GLuint of a texture for the font to use, created and parameterized by you
	// Don't be changing this textures size or anything
	Shikoba::Font * font = new Shikoba::Font(fontface, id_of_gl_texture, 32.0);

	// Create a string of text
	Shikoba::Font * string = new Shikoba::Font(font, (const uint8_t *) "The UTF-8 String.");

	// Create an adjusted string of text
	// See Shikoba.hpp for more details.
	Shikoba::Font * adjusted_string = new Shikoba::Font(font,
		(const uint8_t *) "The UTF-8 String.",
		1.0,				# Character spacing
		1.5,				# Line spacing
		Shikoba::CENTER,	# Alignment
		120,				# Page width (0 = no wrapping)
		4					# Number of spaces a tab makes up
	);

	// Draw the text
	// You set the active texture and vertex array before calling
	// 0 and 1 are the attributes to use for vertex positions and texcoords
	// All strings are drawn starting from (0, 0)
	string->draw(0, 1)
