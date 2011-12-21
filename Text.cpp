#define WITH_GRAPHICS
#define WITH_SOUND
#define WITH_NETWORK
//#define WITH_PYTHON
#include <scge/scge.hpp>

#include <sstream>

int main(int argc, char** argv) {
	scge::window("Hello World", 1680/2, 1050/2, false, true);
	scge::vsync();
	
	// Shaders
	
	scge::shader vshader("vertex", "#version 330 core\n\
	in vec2 vertex;\n\
	in vec2 texcoords;\n\
	in vec4 ink;\n\
	\n\
	uniform mat4 matrix;\n\
	\n\
	out vec2 texcoord;\n\
	out vec4 inkcolor;\n\
	\n\
	void main() {\n\
		texcoord = texcoords;\n\
		inkcolor = ink;\n\
	\n\
		gl_Position = matrix * vec4(vertex, 0.0, 1.0);\n\
	}");
	
	vshader.compile();
	
	scge::shader fshader("fragment", "#version 330 core\n\
	in vec2 texcoord;\n\
	in vec4 inkcolor;\n\
	\n\
	uniform sampler2D tex;\n\
	\n\
	out vec4 color;\n\
	\n\
	void main() {\n\
		color = vec4(1.0, 1.0, 1.0, texture(tex, texcoord).r) * inkcolor;\n\
	}");
	
	fshader.compile();
	
	scge::program fontprogram = scge::program();
	
	fontprogram.attach(&vshader);
	fontprogram.attach(&fshader);
	
	fontprogram.attribute(0, "vertex");
	fontprogram.attribute(1, "texcoords");
	fontprogram.attribute(2, "ink");
	
	fontprogram.link();
	
	scge::use_program(&fontprogram);
	
	// Font
	Shikoba::Library fontlib = Shikoba::Library();
	Shikoba::Face face1 = Shikoba::Face(&fontlib, "/usr/share/fonts/truetype/freefont/FreeSerif.ttf");
	
	// Setup
	
	scge::vao vao = scge::vao();
	
	scge::use_vao(&vao);
	
	vao.enable(0);
	vao.enable(1);
	vao.enable(2);
	
	Shikoba::Font * font1 = new Shikoba::Font(&face1, 32.0);
	
	float fontsize = 100.0;
	
	// Loop
	
	scge::clear_color(0., .0, .0);
	scge::enable("cull", false);
	
	scge::vbo V = scge::vbo(2258 * 4 * 4 * sizeof(GLfloat), "static draw");
	
	{
		scge::use_vao(&vao);
		std::vector<GLfloat> vdas;
		for(int i = 0; i < 2258; i++) {
			vdas.push_back((GLfloat).6);
			vdas.push_back((GLfloat).3);
			vdas.push_back((GLfloat).0);
			vdas.push_back((GLfloat)1.);
			
			vdas.push_back((GLfloat)1);
			vdas.push_back((GLfloat)1);
			vdas.push_back((GLfloat)1);
			vdas.push_back((GLfloat)1.);
			
			vdas.push_back((GLfloat)1);
			vdas.push_back((GLfloat)1);
			vdas.push_back((GLfloat)1);
			vdas.push_back((GLfloat)1.);
			
			vdas.push_back((GLfloat).6);
			vdas.push_back((GLfloat).3);
			vdas.push_back((GLfloat).0);
			vdas.push_back((GLfloat)1.);
		}
	
		V.data(0, 2258 * 4 * 4 * sizeof(GLfloat), &vdas[0]);
	}
	
	vao.attribute(2, &V, "float", 4);
	
	glm::ivec2 wd = scge::window_dimensions();
	
	Shikoba::Text * string = NULL;
	
	scge::image III = scge::image("../bin/Garden.jpg");
	
	float charspace = 2;
	while(!scge::key("escape") && scge::window_opened()) {
		scge::clear();
		wd = scge::window_dimensions();
		scge::viewport(0, 0, wd.x, wd.y);
		//fontprogram.uniform("matrix", glm::gtc::matrix_transform::translate(glm::ortho((float)0.0, (float)wd.x, (float)0.0, (float)wd.y, (float)-1.0, (float)1.0), glm::vec3((float) 30.0, (float) 30.0, (float) 0.0)));
		//fontprogram.uniform("matrix", glm::ortho((float)0.0, (float)1.0, (float)0.0, (float)1.0, (float)-1.0, (float)1.0));
		
		//scge::use_vao(&vao);
		//scge::use_vbo(&V);
		//vao.attribute(0, &V, "float", 2, 0);
		//vao.attribute(1, &V, "float", 2, 8 * sizeof(GLfloat));
		//glBindTexture(GL_TEXTURE_2D, font1._textures.back());
		//fontprogram.uniform("tex", 0);
		//scge::draw("triangle fan", 4);
		
		//fontprogram.uniform("matrix", glm::gtc::matrix_transform::scale(glm::gtc::matrix_transform::translate(glm::ortho((float)0.0, (float)wd.x, (float)0.0, (float)wd.y, (float)-1.0, (float)1.0), glm::vec3((float) 70.0, (float) 70.0, (float) 0.0)), glm::vec3((float) 13.0, (float) 13.0, (float) 0)));
		if(string)
		for(int l = 0; l < 1; l++) {
			fontprogram.uniform("matrix", glm::gtc::matrix_transform::translate(glm::ortho((float)0.0, (float)wd.x, (float)0.0, (float)wd.y, (float)-1.0, (float)1.0), glm::vec3((float) wd.x / 2.0, (float) wd.y - font1->height() - font1->height() * l * (string->lines()), (float) 0.0)));
			string->draw(0, 1);
		}
		
		GLenum err = glGetError();
		if(err)
			printf("0x%x\n", err);
		scge::swap();
		scge::poll();
		
		if(!font1 || scge::key("z")) {
			fontsize += scge::key("shift") ? -1 : 1;
			if(font1)
				delete font1;
			
			printf("%f\n", fontsize);
			font1 = new Shikoba::Font(&face1, fontsize);
			printf("%f\n", fontsize);
		}
		
		if(!string || scge::key("space") || scge::key("z")) {
			if(scge::key("space"))
				charspace += scge::key("shift") ? -0.1 : 0.1;
			if(string)
				delete string;
			
			string = new Shikoba::Text(font1, (const uint8_t *)"Hi",
	1, 1, Shikoba::LEFT, charspace * 100, 8);
		}
	}
}
