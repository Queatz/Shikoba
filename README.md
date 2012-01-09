Shikoba is a text library for OpenGL 3 and above using the FreeType 2 library.

-- Quickstart --

	// Create a library
	Shikoba::Library * fontlib = new Shikoba::Library();

	// Create a face in the library
	Shikoba::Font * fontface = new Shikoba::Face(fontlib, "fontfile.ttf");

	// Create a sized font out of a face
	// id_of_gl_texture is a GLuint of a texture for the font to use created and parameterized by you
	// Don't be changing this textures size or anything
	Shikoba::Font * font = new Shikoba::Font(fontface, id_of_gl_texture, 32.0);

	// Create a string of text
	Shikoba::Font * string = new Shikoba::Font(font, "The UTF-8 String.");

	// Create an adjusted string of text
	Shikoba::Font * adjusted_string = new Shikoba::Font(font, "The UTF-8 String.", character_spacing, line_spacing, alignment, page_width, tab_space_count);

	// Draw the text
	// 0 and 1 are the attributes to use for vertex positions and texcoords
	// All strings are drawn starting from (0, 0)
	string->draw(0, 1)
