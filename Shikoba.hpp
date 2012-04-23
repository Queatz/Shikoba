#ifdef _WIN32
typedef unsigned int uint32_t;
#include <GL/glew.h>
#else
#ifndef GL3_PROTOTYPES
#define GL3_PROTOTYPES
#endif
#include <GL3/gl3.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

#include <vector>
#include <map>

namespace Shikoba {

struct Library;
struct Face;

struct Box {
	GLfloat x1;
	GLfloat y1;
	GLfloat x2;
	GLfloat y2;
};

struct Glyph {
	Box vertices;
	Box texcoords;
	FT_Glyph_Metrics _ft_metrics;
	FT_Vector _ft_advance;
	FT_UInt _glyphid;
};

/* Face */

struct Face {
	/***
	Create a face from a font file.
		const char * | File.
		[unsigned int] | Face in the file.
	***/
	Face(Library *, const char *, unsigned int = 0);
	~Face();
	
	void size(unsigned int);
	const Glyph * glyph(const uint32_t);
	float advance(const uint32_t, const uint32_t = 0);
	float height();
	float ascender();
	float descender();
	
	private:
	
	unsigned int _size;
	FT_Face _ft_face;
	std::map<unsigned int, std::map<uint32_t, Glyph> > _glyphs;
	Library * _library;

	friend struct Library;
};

/* Library */

struct Context {
	GLuint maximum_texture_size;
};

struct Library {
	/***
	Create a library.
	***/
	Library();
	~Library();

	GLuint texture();

	private:
	
	FT_Library _ft_library;
	Context _context;
	GLuint _texid;

	GLint _texturepen_x;
	GLint _texturepen_y;
	GLint _texturerow_h;
	GLint _texture_width;
	GLint _texture_height;
	
	friend struct Face;
};

} // namespace Shikoba
