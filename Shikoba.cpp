#include "Shikoba.hpp"
#include <cmath>

namespace Shikoba {

Face::Face(Library * lib, const char * filepath, unsigned int index) {
	_library = lib;
	
	FT_Error err;
	err = FT_New_Face(_library->_ft_library, filepath, index, &_ft_face);

	if(err) {
		printf("Shikoba: Could not load (sad) face. :(\n");
		return;
	}
}

Face::~Face() {
	FT_Done_Face(_ft_face);
}

void Face::size(unsigned int s) {
	_size = s;
	FT_Set_Char_Size(_ft_face, _size * 64.0, _size * 64.0, 0, 0);
}

const Glyph * Face::glyph(const uint32_t c) {
	if(_glyphs[_size].count(c) == 0) {
		FT_Error err;

		err = FT_Load_Char(_ft_face, (const FT_UInt)c, FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT);
		
		if(err) {
			printf("Shikoba: Char load fail!\n");
			return NULL;
		}

		GLint last;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last);
		
		if((GLuint)last != _library->_texid)
			glBindTexture(GL_TEXTURE_2D, _library->_texid);
		
		// Insert into texture
		
		if(_library->_texture_width == 0 || _library->_texture_height == 0) {
			// Create texture

			_library->_texture_width = 512;
			_library->_texture_height = 512;

			glGetError();
			GLubyte* blackpixels;
			blackpixels = new GLubyte[_library->_texture_width * _library->_texture_height];
			memset(blackpixels, 0, _library->_texture_width * _library->_texture_height);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, _library->_texture_width, _library->_texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, blackpixels);
			glGenerateMipmap(GL_TEXTURE_2D);
			GLenum e;
			e = glGetError();
			if(e) {
				printf("Shikoba: Could not format texture: %u\n", e);
			}
			delete blackpixels;

			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &_library->_texture_width);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &_library->_texture_height);

			if(_library->_texture_width == 0 || _library->_texture_height == 0) {
				printf("Shikoba: Could not found texture! %u\n", glGetError());
				return NULL;
			}
			
			// All glyphs have a 1 pixel padding
			_library->_texturepen_x = 1;
			_library->_texturepen_y = 1;
			
			// Tallest glyph in the current row
			// Could check all glyphs in row and pack better...
			_library->_texturerow_h = 0;
		}
		
		// Shortcut
		FT_Bitmap * bitmap = &_ft_face->glyph->bitmap;
		
		if(_library->_texturepen_x + bitmap->width > _library->_texture_width) {
			_library->_texturepen_y += _library->_texturerow_h + 1;
			_library->_texturepen_x = 1;
			
			if(_library->_texturepen_y + bitmap->rows > _library->_texture_height - 1) {
				printf("Shikoba: Texture overflow...FIXME\n");
			}
		}
		
		GLubyte * data;
		int dsize = bitmap->width * bitmap->rows;
		data = new GLubyte[dsize];
	
		int x, y, i = 0;
		for(y = 0; y < bitmap->rows; y++) 
		for(x = 0; x < bitmap->width; x++)
			data[x + (bitmap->rows - 1 - y) * bitmap->width] = bitmap->buffer[i++];
		
		GLint uplast;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &uplast);
		if(uplast != 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexSubImage2D(GL_TEXTURE_2D, 0, _library->_texturepen_x, _library->_texturepen_y, bitmap->width, bitmap->rows, GL_RED, GL_UNSIGNED_BYTE, data);
		
		delete [] data;
		
		if(uplast != 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, uplast);
		
		if((GLuint)last != _library->_texid)
			glBindTexture(GL_TEXTURE_2D, last);
		
		Glyph * g = &_glyphs[_size][c];
		
		g->_glyphid = FT_Get_Char_Index(_ft_face, (const FT_ULong)c);
		
		g->_ft_metrics = _ft_face->glyph->metrics;
		g->_ft_advance = _ft_face->glyph->advance;
		
		g->vertices.x1 = (float)_ft_face->glyph->bitmap_left - .5;
		g->vertices.y1 = -((float)bitmap->rows - (float)_ft_face->glyph->bitmap_top) - .5;
		g->vertices.x2 = (float)bitmap->width + (float)_ft_face->glyph->bitmap_left + .5;
		g->vertices.y2 = -((float)bitmap->rows - (float)_ft_face->glyph->bitmap_top) + (float)bitmap->rows + .5;
		
		g->texcoords.x1 = (((float)_library->_texturepen_x) - .5) / (float)_library->_texture_width;
		g->texcoords.y1 = (((float)_library->_texturepen_y) - .5) / (float)_library->_texture_height;
		g->texcoords.x2 = ((float)(_library->_texturepen_x + bitmap->width) + .5) / (float)_library->_texture_width;
		g->texcoords.y2 = ((float)(_library->_texturepen_y + bitmap->rows) + .5) / (float)_library->_texture_height;
		
		// Ready for next character when needed
		
		_library->_texturepen_x += bitmap->width + 1;
		
		if(bitmap->rows > _library->_texturerow_h)
			_library->_texturerow_h = bitmap->rows;
	}
	
	return &_glyphs[_size][c];
}

float Face::advance(const uint32_t lg, const uint32_t rg) {
	if(rg && FT_HAS_KERNING(_ft_face)) {
		FT_Vector kern;
		FT_Get_Kerning(_ft_face, lg, rg, FT_KERNING_DEFAULT, &kern);
		return (float)(glyph(lg)->_ft_advance.x >> 6) + (float)(kern.x >> 6);
	}
	else
		return (float)(glyph(lg)->_ft_advance.x >> 6);
}

float Face::height() {
	return (float)(_ft_face->size->metrics.height >> 6);
}

float Face::ascender() {
	return (float)(_ft_face->size->metrics.ascender >> 6);
}

float Face::descender() {
	return (float)(_ft_face->size->metrics.descender >> 6);
}


Library::Library() : _texid(0), _texture_width(0), _texture_height(0) {
	FT_Error err;
	err = FT_Init_FreeType(&_ft_library);

	if(err) {
		printf("Shikoba: Could not initialize library.\n");
	}
	
	GLint i;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
	_context.maximum_texture_size = (GLuint)i;
	
	if(_context.maximum_texture_size == 0)
		printf("Shikoba: Invalid context! Be wary!");
		
	glGetError();

	glGenTextures(1, &_texid);

	GLenum e;
	e = glGetError();
	if(e) {
		printf("Shikoba: Could not create a texture: %u\n", e);
	}
}

Library::~Library() {
	if(_ft_library) {
		FT_Error err;
		err = FT_Done_FreeType(_ft_library);

		glDeleteTextures(1, &_texid);

		if(err) {
			printf("Shikoba: Could not historate library.\n");
		}
	}
}

GLuint Library::texture() { return _texid; }

} // namespace Shikoba
