Font::Font(Face *face, GLuint texID, float width, float height, int horizontal_dpi, int vertical_dpi) {
	_face = face;
	
	FT_Error err;
	err = FT_New_Size(_face->_ft_face, &_ft_size);
	
	if(err) {
		printf("Invalid font!\n");
		return;
	}
	
	FT_Activate_Size(_ft_size);
	
	err = FT_Set_Char_Size(_face->_ft_face, width * 64.0, height * 64.0, horizontal_dpi, vertical_dpi);
	
	if(err) {
		printf("Invalid font size!\n");
		return;
	}
	
	GLint last;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last);
	
	if((GLuint) last != texID)
		glBindTexture(GL_TEXTURE_2D, texID);
	
	if(!_face->_ft_face->size) {
		printf("Invalid font size the second generation!\n");
		return;
	}
	
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &_texture_width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &_texture_height);
	
	if(_texture_width == 0 || _texture_height == 0) {
		printf("Invalid texture!\n");
		return;
	}
	
	GLubyte* blackpixels;
	blackpixels = new GLubyte[_texture_width * _texture_height];
	memset(blackpixels, 0, _texture_width * _texture_height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _texture_width, _texture_height, GL_RED, GL_UNSIGNED_BYTE, blackpixels);
	delete blackpixels;
	
	if((GLuint) last != texID)
		glBindTexture(GL_TEXTURE_2D, last);
	
	// All glyphs have a 1 pixel padding
	_texturepen_x = 1;
	_texturepen_y = 1;
	
	// Tallest glyph in the current row
	// Could check all glyphs in row and pack better...
	_texturerow_h = 0;
	
	_textures.push_back(texID);
}

Font::~Font() {
	if(!_textures.empty())
		glDeleteTextures(_textures.size(), &_textures[0]);
	
	FT_Done_Size(_ft_size);
}

float Font::height() {
	return (float)(_ft_size->metrics.height >> 6);
}

float Font::ascender() {
	return (float)(_ft_size->metrics.ascender >> 6);
}

float Font::descender() {
	return (float)(_ft_size->metrics.descender >> 6);
}

float Font::advance(const unsigned char * utf8) {
	FT_Error err;
	err = FT_Activate_Size(_ft_size);

	const unsigned char * i = utf8;
	uint32_t c;
	
	const Glyph * temp;
	const Glyph * lasttemp = 0;
	
	GLfloat pen_x = 0.0;
	
	FT_Vector kern;
	int has_kerning = FT_HAS_KERNING(_face->_ft_face);
	
	while(*i != 0) {
		c = utf8::unchecked::next(i);
		temp = glyph(c);
		
		if(has_kerning && lasttemp != NULL) {
			err = FT_Get_Kerning(_face->_ft_face, lasttemp->_glyphid, temp->_glyphid, FT_KERNING_DEFAULT, &kern);
			if(err == 0)
				pen_x += (float)(kern.x >> 6);
		}
		
		pen_x += (float)(temp->_ft_advance.x >> 6);
		
		lasttemp = temp;
	}
	
	return pen_x;
}

const Glyph * Font::glyph(const uint32_t & c) {
	if(_glyphs.count(c) == 0) {
		FT_Error err;
		
		err = FT_Load_Char(_face->_ft_face, (const FT_UInt)c, FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT);
		
		if(err) {
			printf("Char load fail!\n");
			return NULL;
		}
		
		// Insert into texture
		
		if(_textures.empty()) {
			printf("No texture!\n");
			return NULL;
		}
		
		GLint last;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last);
		
		if((GLuint)last != _textures.back())
			glBindTexture(GL_TEXTURE_2D, _textures.back());
		
		// Shortcut
		FT_Bitmap * bitmap = &_face->_ft_face->glyph->bitmap;
		
		if(_texturepen_x + bitmap->width > _texture_width) {
			_texturepen_y += _texturerow_h + 1;
			_texturepen_x = 1;
			
			if(_texturepen_y + bitmap->rows > _texture_height - 1) {
				printf("Texture overflow...\n");
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

		glTexSubImage2D(GL_TEXTURE_2D, 0, _texturepen_x, _texturepen_y, bitmap->width, bitmap->rows, GL_RED, GL_UNSIGNED_BYTE, data);
		
		delete [] data;
		
		if(uplast != 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, uplast);
		
		if((GLuint)last != _textures.back())
			glBindTexture(GL_TEXTURE_2D, last);
		
		_glyphs[c].tex = 0;
		
		_glyphs[c]._glyphid = FT_Get_Char_Index(_face->_ft_face, (const FT_ULong)c);
		
		_glyphs[c]._ft_metrics = _face->_ft_face->glyph->metrics;
		_glyphs[c]._ft_advance = _face->_ft_face->glyph->advance;
		
		_glyphs[c].vertices[0] = (float)_face->_ft_face->glyph->bitmap_left - .5;
		_glyphs[c].vertices[1] = -((float)bitmap->rows - (float)_face->_ft_face->glyph->bitmap_top) - .5;
		_glyphs[c].vertices[2] = (float)bitmap->width + (float)_face->_ft_face->glyph->bitmap_left + .5;
		_glyphs[c].vertices[3] = -((float)bitmap->rows - (float)_face->_ft_face->glyph->bitmap_top) + (float)bitmap->rows + .5;
		
		_glyphs[c].texcoords[0] = (((float)_texturepen_x) - .5) / (float)_texture_width;
		_glyphs[c].texcoords[1] = (((float)_texturepen_y) - .5) / (float)_texture_height;
		_glyphs[c].texcoords[2] = ((float)(_texturepen_x + bitmap->width) + .5) / (float)_texture_width;
		_glyphs[c].texcoords[3] = ((float)(_texturepen_y + bitmap->rows) + .5) / (float)_texture_height;
		
		// Ready for next character when needed
		
		_texturepen_x += bitmap->width + 1;
		
		if(bitmap->rows > _texturerow_h)
			_texturerow_h = bitmap->rows;
	}
	
	return &_glyphs[c];
}
