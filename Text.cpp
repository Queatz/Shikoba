struct _lineglyph {
	_lineglyph(const Glyph * g, int a, int b) : glyph(g), x(a), y(b) {}
	
	const Glyph * glyph;
	int x, y;
};

Text::Text(Font * font, const unsigned char * utf8, float spacing, float linespacing, int align, float maxwidth, int tabwidth) {
	_font = font;
	
	_maxlineadvance = 0.0;
	
	FT_Error err;
	err = FT_Activate_Size(_font->_ft_size);
	
	if(err) {
		printf("Bad size.\n");
		return;
	}

	const unsigned char * i = utf8;
	const unsigned char * i2;
	const unsigned char * i3;
	uint32_t c = 0;
	uint32_t clast = 0;
	
	const Glyph * temp;
	const Glyph * lasttemp = 0;
	
	std::vector<GLushort> indicedata;
	std::vector<GLfloat> pendata;
	
	GLfloat pen_x = 0.0;
	GLfloat pen_y = 0.0;
	GLfloat line_pen_x = 0.0;
	GLfloat break_pen_x = 0.0;
	
	GLsizei characters = 0;
	
	FT_Vector kern;
	int has_kerning = FT_HAS_KERNING(_font->_face->_ft_face);
	
	int lines = 0;
	
	err = FT_Load_Char(_font->_face->_ft_face, (const FT_UInt)' ', FT_LOAD_DEFAULT | FT_LOAD_IGNORE_TRANSFORM);
	
	int spaceadvance = (_font->_face->_ft_face->glyph->advance.x >> 6);
	tabwidth *= (float)spaceadvance;
	
	std::vector<_lineglyph> lineglyphs;
	
	bool linebroke = false;
	int current_word_length;
	
	char ending_sequence = 0;
	
	while(1) {
		if(ending_sequence == 1) {
			ending_sequence = 2;
			goto doline;
		}
		else if(ending_sequence == 2)
			break;
		
		i2 = i;
		clast = c;
		c = utf8::unchecked::next(i);
		if(*i == 0)
			ending_sequence = 1;
		
		temp = _font->glyph(c);
		
		if(c == '\n' || c == '\r') {
			if(clast == '\r' && c == '\n')
				continue;
			
			lasttemp = NULL;
			current_word_length = 0;
		
			linebroke = true;
			goto doline;
		}
		else if(c == '\t') {
			lasttemp = NULL;
			current_word_length = 0;
			if(clast != '\t' && clast != ' ')
				break_pen_x = line_pen_x;
			pen_x = (float)((int)((pen_x + tabwidth) / tabwidth)) * tabwidth * spacing;
			i3 = i;
			continue;
		}
		else if(c == ' ') {
			lasttemp = NULL;
			current_word_length = 0;
			if(clast != '\t' && clast != ' ')
				break_pen_x = line_pen_x;
			pen_x += (float)spaceadvance * spacing;
			i3 = i;
			continue;
		}
		
		if(has_kerning && lasttemp != NULL) {
			err = FT_Get_Kerning(_font->_face->_ft_face, lasttemp->_glyphid, temp->_glyphid, FT_KERNING_DEFAULT, &kern);
			if(err == 0)
				pen_x += (float)(kern.x >> 6);
		}
		
		if(maxwidth > 0.0 && !lineglyphs.empty() && pen_x + (float)(temp->_ft_advance.x >> 6) >= maxwidth) {
			if(break_pen_x > 0.001) {
				for(int ci = 0; ci < current_word_length; ci++)
					lineglyphs.pop_back();
			
				i = i3;
				line_pen_x = break_pen_x;
			} else {
				i = i2;
			}
		
			goto doline;
		}
		
		lineglyphs.push_back(_lineglyph(temp, pen_x, pen_y));
		
		line_pen_x = pen_x + (float)(temp->_ft_advance.x >> 6);
		pen_x += (float)(temp->_ft_advance.x >> 6) * spacing;
		current_word_length += 1;
		lasttemp = temp;
		
		continue;
		doline:
		
		float lineshift, linestretch;
		
		if(maxwidth > 0.0 && !linebroke && align & JUSTIFY) {
			linestretch = maxwidth / line_pen_x;
		}
		else {
			linestretch = 1.0;
		}
		
		if(align & LEFT) {
			lineshift = 0.0;
		}
		else if(align & RIGHT) {
			lineshift = std::floor(line_pen_x * linestretch);
		}
		else if(align & CENTER) {
			lineshift = std::floor(line_pen_x * linestretch / 2.0);
		}
		
		if(_maxlineadvance < line_pen_x * linestretch)
			_maxlineadvance = line_pen_x * linestretch;
		
		std::vector<_lineglyph>::iterator q;
		for(q = lineglyphs.begin(); q != lineglyphs.end(); q++) {
			pendata.push_back(std::floor((*q).x * linestretch) + (*q).glyph->vertices[0] - lineshift);
			pendata.push_back((*q).y + (*q).glyph->vertices[1]);
			pendata.push_back((*q).glyph->texcoords[0]);
			pendata.push_back((*q).glyph->texcoords[1]);
		
			pendata.push_back(std::floor((*q).x * linestretch) + (*q).glyph->vertices[0] - lineshift);
			pendata.push_back((*q).y + (*q).glyph->vertices[3]);
			pendata.push_back((*q).glyph->texcoords[0]);
			pendata.push_back((*q).glyph->texcoords[3]);
		
			pendata.push_back(std::floor((*q).x * linestretch) + (*q).glyph->vertices[2] - lineshift);
			pendata.push_back((*q).y + (*q).glyph->vertices[3]);
			pendata.push_back((*q).glyph->texcoords[2]);
			pendata.push_back((*q).glyph->texcoords[3]);
		
			pendata.push_back(std::floor((*q).x * linestretch) + (*q).glyph->vertices[2] - lineshift);
			pendata.push_back((*q).y + (*q).glyph->vertices[1]);
			pendata.push_back((*q).glyph->texcoords[2]);
			pendata.push_back((*q).glyph->texcoords[1]);
		
			indicedata.push_back(characters * 4 + 0);
			indicedata.push_back(characters * 4 + 1);
			indicedata.push_back(characters * 4 + 2);
			indicedata.push_back(characters * 4 + 0);
			indicedata.push_back(characters * 4 + 2);
			indicedata.push_back(characters * 4 + 3);
			
			characters++;
		}
		
		lines += 1;
		pen_x = 0.0;
		pen_y -= (float)(_font->_ft_size->metrics.height >> 6) * linespacing;
		break_pen_x = pen_x;
		current_word_length = 0;
		
		linebroke = false;
		
		lineglyphs.clear();
	}
	
	_lines = lines;
	_characters = characters;
	GLint last, last2;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last2);
	
	glGenBuffers(2, _buffers);
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, _characters * 4 * (2 + 2) * sizeof(GLfloat), (const GLvoid *)&pendata[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _characters * sizeof(GLushort) * 6, (const GLvoid *)&indicedata[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, last);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last2);
}

Text::~Text() {
	if(_buffers[0] && _buffers[1])
		glDeleteBuffers(2, _buffers);
}

int Text::lines() {
	return _lines;
}

float Text::advance() {
	return _maxlineadvance;
}

void Text::draw(GLuint vertexattribute, GLuint texcoordattribute) {
	// Texture
	
	GLint lasti;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lasti);
	
	glBindTexture(GL_TEXTURE_2D, _font->_textures.back());
	
	// Buffers
	
	GLint last, last2;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last2);
	
	// Pen
	if((GLuint)last != _buffers[0])
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	
	// Vertex + Texcoord
	
	glVertexAttribPointer(vertexattribute, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const GLvoid *)0);
	glVertexAttribPointer(texcoordattribute, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const GLvoid *)(sizeof(GLfloat) * 2));
	
	if((GLuint)last2 != _buffers[1])
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[1]);
	
	// Draw
	glDrawElements(GL_TRIANGLES, _characters * 6, GL_UNSIGNED_SHORT, (const GLvoid *)0);
	
	// Restore
	glBindBuffer(GL_ARRAY_BUFFER, last);
	
	if((GLuint)last2 != _buffers[1])
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last2);
	
	glBindTexture(GL_TEXTURE_2D, lasti);
}
