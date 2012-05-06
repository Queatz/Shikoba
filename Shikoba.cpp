#include "Shikoba.hpp"
#include <cmath>

namespace Shikoba {

Face::Face(Library * lib, const char * filepath, unsigned int index) {
	_library = lib;
	
	FT_Error err;
	err = FT_New_Face(_library->_ft_library, filepath, index, &_ft_face);

	if(err) {
		_library->_errorString = "Could not load face.\n";
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
			_library->_errorString = "Char load failed.\n";
			return NULL;
		}

		GLint last;
		glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &last);
		
		if((GLuint)last != _library->_texid)
			glBindTexture(GL_TEXTURE_RECTANGLE, _library->_texid);
		
		if(_library->_texture_width == 0 || _library->_texture_height == 0) {
			if(!_library->expandTexture())
				return NULL;
		}
		
		FT_Bitmap * bitmap = &_ft_face->glyph->bitmap;
		
		if(_library->_texturepen_x + bitmap->width + 1 > _library->_texture_width) {
			_library->_texturepen_y += _library->_texturerow_h + 1;
			_library->_texturepen_x = 1;
			_library->_texturerow_h = 0;
			
			if(_library->_texturepen_y + bitmap->rows + 1 > _library->_texture_height) {
				_library->expandTexture();
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

		glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, _library->_texturepen_x, _library->_texturepen_y, bitmap->width, bitmap->rows, GL_RED, GL_UNSIGNED_BYTE, data);
		
		delete [] data;
		
		if(uplast != 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, uplast);
		
		if((GLuint)last != _library->_texid)
			glBindTexture(GL_TEXTURE_RECTANGLE, last);
		
		Glyph * g = &_glyphs[_size][c];
		
		g->_glyphid = FT_Get_Char_Index(_ft_face, (const FT_ULong)c);
		
		g->_ft_metrics = _ft_face->glyph->metrics;
		g->_ft_advance = _ft_face->glyph->advance;
		
		g->vertices[0] = (GLfloat)_ft_face->glyph->bitmap_left - 0.5;
		g->vertices[1] = -(GLfloat)bitmap->rows + (GLfloat)_ft_face->glyph->bitmap_top - 0.5;
		g->vertices[2] = (GLfloat)bitmap->width + (GLfloat)_ft_face->glyph->bitmap_left + 0.5;
		g->vertices[3] = -(GLfloat)bitmap->rows + (GLfloat)_ft_face->glyph->bitmap_top + (GLfloat)bitmap->rows + 0.5;
		
		g->texcoords[0] = (GLfloat)_library->_texturepen_x - 0.5;
		g->texcoords[1] = (GLfloat)_library->_texturepen_y - 0.5;
		g->texcoords[2] = (GLfloat)_library->_texturepen_x + (GLfloat)bitmap->width + 0.5;
		g->texcoords[3] = (GLfloat)_library->_texturepen_y + (GLfloat)bitmap->rows + 0.5;
		
		_library->_texturepen_x += bitmap->width + 1;
		
		if(bitmap->rows > _library->_texturerow_h)
			_library->_texturerow_h = bitmap->rows;
	}
	
	return &_glyphs[_size][c];
}

GLfloat Face::advance(const uint32_t lg, const uint32_t rg) {
	if(rg && FT_HAS_KERNING(_ft_face)) {
		FT_Vector kern;
		FT_Get_Kerning(_ft_face, lg, rg, FT_KERNING_DEFAULT, &kern);
		return (GLfloat)(glyph(lg)->_ft_advance.x >> 6) + (GLfloat)(kern.x >> 6);
	}
	else
		return (GLfloat)(glyph(lg)->_ft_advance.x >> 6);
}

GLfloat Face::height() {
	return (GLfloat)(_ft_face->size->metrics.height >> 6);
}

GLfloat Face::ascender() {
	return (GLfloat)(_ft_face->size->metrics.ascender >> 6);
}

GLfloat Face::descender() {
	return (GLfloat)(_ft_face->size->metrics.descender >> 6);
}


Library::Library() : _texid(0), _texture_width(0), _texture_height(0), _errorString(0) {
	FT_Error err;
	err = FT_Init_FreeType(&_ft_library);

	if(err)
		_errorString = "Could not initialize library.\n";
	
	GLint i;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
	_context.maximum_texture_size = (GLuint)i;
	
	if(_context.maximum_texture_size == 0)
		_errorString = "Invalid context! Be wary!\n";
		
	glGetError();

	glGenTextures(1, &_texid);

	GLenum e;
	e = glGetError();
	if(e)
		_errorString = "Could not create a texture.\n";
}

Library::~Library() {
	if(_ft_library) {
		FT_Error err;
		err = FT_Done_FreeType(_ft_library);

		glDeleteTextures(1, &_texid);

		if(err)
			_errorString = "Could not historate library.\n";
	}
}

GLuint Library::texture() { return _texid; }

GLboolean Library::expandTexture() {
	if(_texture_height == _context.maximum_texture_size) {
		_errorString = "Texture overflow.\n";
		return GL_FALSE;
	}
	
	if(_texture_width == 0 || _texture_height == 0) {
		_texture_width = _context.maximum_texture_size;
		_texture_height = 128;

		glGetError();
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R8, _texture_width, _texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		GLenum e;
		e = glGetError();
		if(e)
			_errorString = "Could not format texture.\n";

		glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE, 0, GL_TEXTURE_WIDTH, &_texture_width);
		glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE, 0, GL_TEXTURE_HEIGHT, &_texture_height);

		if(_texture_width == 0 || _texture_height == 0) {
			_errorString = "Could not create texture.\n";
			return GL_FALSE;
		}
	
		_texturepen_x = 1;
		_texturepen_y = 1;
		_texturerow_h = 0;
	}
	else {
		GLubyte * cbuf = new GLubyte[_texture_width * _texture_height];
		
		glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_RED, GL_UNSIGNED_BYTE, cbuf);
		
		GLint th = _texture_height;
		_texture_height *= 2;
		if(_texture_height > _context.maximum_texture_size)
			_texture_height = _context.maximum_texture_size;
		
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R8, _texture_width, _texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, _texture_width, th, GL_RED, GL_UNSIGNED_BYTE, cbuf);

		glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE, 0, GL_TEXTURE_WIDTH, &_texture_width);
		glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE, 0, GL_TEXTURE_HEIGHT, &_texture_height);
		
		delete [] cbuf;
	}
	
	return GL_TRUE;
}

const char * Library::getErrorString() {
	const char * err = _errorString;
	_errorString = NULL;
	return err;
}

} // namespace Shikoba
