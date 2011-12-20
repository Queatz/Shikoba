Library::Library() {
	FT_Error err;
	err = FT_Init_FreeType(&_ft_library);

	if(err) {
		printf("Could not initialize library.\n");
	}
	
	GLint i;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
	_context.maximum_texture_size = (GLuint)i;
	
	if(_context.maximum_texture_size == 0)
		printf("Invalid context! Be wary!");
}

Library::~Library() {
	if(_ft_library) {
		FT_Error err;
		err = FT_Done_FreeType(_ft_library);
	
		if(err) {
			printf("Could not historate library.\n");
		}
	}
}
