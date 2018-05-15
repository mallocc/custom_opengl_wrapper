#include <stdio.h>

#include "gl_wrapper.h"

#include "glslprogram.h"
#include "Obj.h"
#include "colors.h"
#include "fbo.h"
#include "light.h"

#include <vector>;

GLSLProgramManager	program_manager;

GLContent content;

glm::vec3
ambient_color;

Obj
screen_texture,
sphere;

FBO
basic_fbo;

GLSLProgramID 
BASIC_PROGRAM, 
PHONG_PROGRAM;

Light light = { glm::vec3(0,5,-5), WHITE, glm::vec3(1,1,100) };

void init()
{
	//// CREATE GLSL PROGAMS
	printf("\n");
	printf("Initialising GLSL programs...\n");
	BASIC_PROGRAM =
		program_manager.add_program("shaders/basic.vert", "shaders/basic.frag",
		content.get_model_mat(), content.get_view_mat(), content.get_proj_mat());
	PHONG_PROGRAM =
		program_manager.add_program("shaders/phong.vert", "shaders/phong.frag",
			content.get_model_mat(), content.get_view_mat(), content.get_proj_mat());
	program_manager.get_program(PHONG_PROGRAM)
		->add_handle(VarHandle("u_ambient_color", &ambient_color))
		->add_handle(VarHandle("u_light_color", &light.color))
		->add_handle(VarHandle("u_eye_pos", content.get_eye_pos()))
		->add_handle(VarHandle("u_light_pos", &light.pos))
		->add_handle(VarHandle("u_light_properties", &light.brightness_specscale_shinniness));
	//basic_fbo = FBO(window_size.x, window_size.y);

	std::vector<glm::vec3>
		v,
		c,
		n,
		t;

	v = generate_square_mesh(1, 1);

	screen_texture = Obj(
		"", "", "",
		//"textures/moss_color.jpg",
		//"textures/moss_norm.jpg",
		//"textures/moss_height.jpg",
		pack_object(&v, GEN_UVS_RECTS, BLACK),
		glm::vec3(),
		glm::vec3(0, 0, 1),
		glm::radians(90.0f),
		glm::vec3(1, 1, 1)
	);

	//// CREATE OBJECTS
	printf("\n");
	printf("Initialising objects...\n");

	int res = 200;

	v = generate_sphere(res, res);

	sphere = Obj(
		"", "", "",
		pack_object(&v, GEN_COLOR_RAND | GEN_NORMS, WHITE),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0), glm::radians(0.0f),
		glm::vec3(1, 0, 0), glm::radians(90.0f),
		glm::vec3(1, 1, 1)
	);
}

void draw_loop()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.);

	content.loadPerspective();
	
	VarHandle * model_mat_handle =
		program_manager.load_program(PHONG_PROGRAM)
		->get_model_mat4_handle();
	sphere.draw(0, model_mat_handle, nullptr, nullptr, nullptr);
}

int main()
{
	content.run(draw_loop, init);

	return 0;
}
