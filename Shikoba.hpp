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

#define SHIKOBA_VERSION 0.1

namespace Shikoba {

struct Library;
struct Face;

struct Glyph {
	GLfloat vertices[4];
	GLfloat texcoords[4];
	FT_Glyph_Metrics _ft_metrics;
	FT_Vector _ft_advance;
	FT_UInt _glyphid;
};

/* Face */

struct Face {
	Face(Library *, const char *, unsigned int = 0);
	~Face();
	
	void size(unsigned int);
	const Glyph * glyph(const uint32_t);
	GLfloat advance(const uint32_t, const uint32_t = 0);
	GLfloat height();
	GLfloat ascender();
	GLfloat descender();
	
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
	Library();
	~Library();

	GLuint texture();
	const char * getErrorString();

	private:
	
	GLboolean expandTexture();
	
	FT_Library _ft_library;
	Context _context;
	GLuint _texid;
	
	const char * _errorString;

	GLint _texturepen_x;
	GLint _texturepen_y;
	GLint _texturerow_h;
	GLint _texture_width;
	GLint _texture_height;
	
	friend struct Face;
};

} // namespace Shikoba
