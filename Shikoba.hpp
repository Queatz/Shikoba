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

#include <utf8.h>
#include <vector>
#include <map>

namespace Shikoba {

struct Library;
struct Face;
struct Font;
struct Text;

/***
Alignment constants
	LEFT
	RIGHT
	CENTER
	JUSTIFY

JUSTIFY can be |'d with the others: CENTER | JUSTIFY
***/
const int LEFT = 1 << 0;
const int RIGHT = 1 << 1;
const int CENTER = 1 << 2;
const int JUSTIFY = 1 << 3;

struct Glyph {
	FT_Glyph_Metrics _ft_metrics;
	FT_Vector _ft_advance;
	GLfloat vertices[4];
	GLfloat texcoords[4];
	GLuint tex;
	FT_UInt _glyphid;
};

struct Text {
	/***
	Create a string using a font.
		Font * | The font to use.
		const unsigned char * | A Unicode 8-bit string.
		[float] | Character spacing.
		[float] | Line spacing.
		[int] | Alignment.  Can be LEFT, RIGHT, or CENTER, and can be |'d with JUSTIFY.
		[float] | Maximum line width.  0 means unmetered.
		[int] | Number of spaces a tab represents.
	***/
	Text(Font *, const unsigned char *, float = 1.0, float = 1.0, int = LEFT, float = 0.0, int = 4);
	~Text();
	
	/***
	Draw the string.  It will bind the font's texture and set up the vertex and texcoord attributes.
		GLuint | Vertex position attribute (vec2).
		GLuint | Texture coordinate attribute (vec2).
	***/
	void draw(GLuint, GLuint);
	
	/***
	Get the number of lines in the string.
	***/
	int lines();
	
	/***
	Get the maximum line advance.
	***/
	float advance();
	
	private:
	
	int _lines;
	float _maxlineadvance;
	
	Font * _font;
	
	GLuint _buffers[2];
	GLsizei _characters;
};

/* Font */

struct Font {
	/***
	Create a font from a face with a given size.
		Face * | The face to use.
		float | The font size, or horizontal size.
		[float] | Vertical font size.
		[int] | DPI of surface.
		[int] | Vertical DPI.
	***/
	Font(Face *, GLuint, float, float = 0.0, int = 0, int = 0);
	~Font();
	
	/***
	The height of a single line.
	***/
	float height();
	/***
	How far the font extends above the baseline.
	***/
	float ascender();
	/***
	How far the font descends below the baseline.
	***/
	float descender();
	/***
	Calculate the advance of a string.
		const unsigned char * | The string.
	***/
	float advance(const unsigned char *);
	
	/***
	Access a character glyph.
		const ucs4_t & | The character.
	***/
	const Glyph * glyph(const uint32_t &);
	
	private:
	
	FT_Size _ft_size;
	Face * _face;
	
	std::vector<GLuint> _textures;
	
	std::map<uint32_t, Glyph> _glyphs;
	
	GLint _texturepen_x;
	GLint _texturepen_y;
	GLint _texturerow_h;
	GLint _texture_width;
	GLint _texture_height;
	
	friend struct Text;
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
	
	private:
	
	FT_Face _ft_face;
	Library * _library;
	
	friend struct Font;
	friend struct Text;
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
	
	private:
	
	FT_Library _ft_library;
	Context _context;
	
	friend struct Face;
	friend struct Font;
};

} // namespace Shikoba
