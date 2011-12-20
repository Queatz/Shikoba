Face::Face(Library * lib, const char * filepath, unsigned int index) {
	_library = lib;
	
	FT_Error err;
	err = FT_New_Face(_library->_ft_library, filepath, index, &_ft_face);

	if(err) {
		printf("Could not load (sad) face. :(\n");
		return;
	}
}

Face::~Face() {
	FT_Done_Face(_ft_face);
}
