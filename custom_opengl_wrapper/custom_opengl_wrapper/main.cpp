#include <stdio.h>

#include "gl_wrapper.h"

#include "glslprogram.h"
#include "colors.h"
#include "fbo.h"
#include "light.h"
#include "entity.h"
#include "primitive_generators.h"

#include <vector>;

GLSLProgramManager program_manager;

GLContent content;

glm::vec3
ambient_color;

Entity
screen_texture,
sphere, stars;

FBO
basic_fbo,
blur_fbo,
motionblur_fbo0,
motionblur_fbo1;

GLSLProgramID 
BASIC_PROGRAM, 
PHONG_PROGRAM,
PHONG_TEXTURE_PROGRAM,
RENDER_PROGRAM,
BLUR_PROGRAM,
MIX_PROGRAM;

Light light = { glm::vec3(0,5,-5), WHITE, glm::vec3(1,1,100) };

glm::vec3 blur_properties, motionblur_properties;

//Returns random float
inline float		randf()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

void init()
{
	blur_properties = glm::vec3(3 / content.get_window_size().x, 3 / content.get_window_size().y, 15);
	motionblur_properties = glm::vec3(1.0f, 0.9, 1.0f);

	//// CREATE GLSL PROGAMS
	printf("\n");
	printf("Initialising GLSL programs...\n");
	BASIC_PROGRAM =
		program_manager.add_program("shaders/basic.vert", "shaders/basic.frag",
		content.get_model_mat(), content.get_view_mat(), content.get_proj_mat());

	PHONG_PROGRAM =
		program_manager.add_program("shaders/phong.vert", "shaders/phong.frag",
			content.get_model_mat(), content.get_view_mat(), content.get_proj_mat());

	RENDER_PROGRAM =
		program_manager.add_program("shaders/basic_texture.vert", "shaders/basic_texture.frag",
			content.get_model_mat(), content.get_view_mat(), content.get_proj_mat());

	BLUR_PROGRAM =
		program_manager.add_program("shaders/basic_texture.vert", "shaders/basic_texture_glow.frag",
			content.get_model_mat(), content.get_view_mat(), content.get_proj_mat());

	MIX_PROGRAM =
		program_manager.add_program("shaders/basic_texture.vert", "shaders/combine.frag",
			content.get_model_mat(), content.get_view_mat(), content.get_proj_mat());

	PHONG_TEXTURE_PROGRAM =
		program_manager.add_program("shaders/phong_texture.vert", "shaders/phong_texture.frag",
			content.get_model_mat(), content.get_view_mat(), content.get_proj_mat());

	//// ADDING HANDLES TO PROGRAMS
	printf("\n");
	printf("Adding handles to GLSL programs...\n");
	program_manager.get_program(PHONG_PROGRAM)
		->add_handle(VarHandle("u_ambient_color", &ambient_color))
		->add_handle(VarHandle("u_light_color", &light.color))
		->add_handle(VarHandle("u_eye_pos", content.get_eye_pos()))
		->add_handle(VarHandle("u_light_pos", &light.pos))
		->add_handle(VarHandle("u_light_properties", &light.brightness_specscale_shinniness));

	program_manager.get_program(PHONG_TEXTURE_PROGRAM)
		->set_tex_handle()
		->add_handle(VarHandle("u_ambient_color", &ambient_color))
		->add_handle(VarHandle("u_light_color", &light.color))
		->add_handle(VarHandle("u_eye_pos", content.get_eye_pos()))
		->add_handle(VarHandle("u_light_pos", &light.pos))
		->add_handle(VarHandle("u_light_properties", &light.brightness_specscale_shinniness));

	program_manager.get_program(RENDER_PROGRAM)
		->set_tex_handle();

	program_manager.get_program(BLUR_PROGRAM)
		->set_tex_handle()
		->add_handle(VarHandle("u_glow_res", &blur_properties));

	program_manager.get_program(MIX_PROGRAM)
		->set_tex_handle()
		->set_tex1_handle()
		->add_handle(VarHandle("u_mix_properties", &motionblur_properties));

	//// CREATE FBOS
	printf("\n");
	printf("Creating FBOs...\n");
	basic_fbo = FBO(content.get_window_size());
	blur_fbo = FBO(content.get_window_size());
	motionblur_fbo0 = FBO(content.get_window_size());
	motionblur_fbo1 = FBO(content.get_window_size());
	
	//// ADD OBJECTS TO FBOS
	basic_fbo.add_object(&sphere);
	blur_fbo.add_object(&sphere);
	basic_fbo.add_object(&stars);
	
	//// CREATE OBJECTS
	printf("\n");
	printf("Initialising objects...\n");

	std::vector<glm::vec3> v, c, n, t;

	v = generate_square_mesh(1, 1);

	screen_texture = Entity(
		"",
		pack_object(&v, GEN_UVS_RECTS, BLACK),
		glm::vec3(),
		glm::vec3(0, 0, 1),	glm::radians(90.0f),
		glm::vec3(1, 0, 0),	glm::radians(0.0f),
		glm::vec3(1, 1, 1)
	);

	int res = 200;
	v = generate_sphere(res, res);

	sphere = Entity(
		"textures/mars.jpg",
		pack_object(&v, GEN_UVS_SPHERE | GEN_NORMS, WHITE),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0), glm::radians(0.0f),
		glm::vec3(1, 0, 0), glm::radians(90.0f),
		glm::vec3(1, 1, 1)
	);

	v = generate_sphere_invert(20, 20);

	stars = Entity(
		"textures/stars.jpg",
		pack_object(&v, GEN_UVS_SPHERE | GEN_NORMS, BLACK),
		glm::vec3(0, 0, 0),
		glm::vec3(1, 0, 0),	glm::radians(0.0f),
		glm::vec3(1, 0, 0),	glm::radians(90.0f),
		glm::vec3(1, 1, 1) * 100.0f
	);
}

void physics()
{
	//light.pos = glm::quat(glm::vec3(0, glm::radians(1.0f), 0)) * light.pos;
	sphere.theta += glm::radians(0.1f);
}

void draw_loop()
{
	physics();

	FBO::unbind();

	VarHandle 
		*model_mat_handle,
		*texture_handle,
		*texture1_handle;

	content.clearAll();
	content.loadPerspective();
	program_manager.load_program(PHONG_TEXTURE_PROGRAM);
	model_mat_handle = program_manager.get_current_program()->get_model_mat4_handle();
	texture_handle = program_manager.get_current_program()->get_tex_handle();
	basic_fbo.binding_draw_objects(model_mat_handle, texture_handle);

	content.clearAll();
	content.loadOrtho();
	program_manager.load_program(RENDER_PROGRAM);
	model_mat_handle = program_manager.get_current_program()->get_model_mat4_handle();
	texture_handle = program_manager.get_current_program()->get_tex_handle();
	basic_fbo.activate_texture(texture_handle);
	screen_texture.draw(0, model_mat_handle, nullptr);
	basic_fbo.deactivate_texture();
	
}

static void		key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == 2)
	{
		switch (key)
		{
		case GLFW_KEY_UP:
			motionblur_properties.y += 0.01f;
			break;
		case GLFW_KEY_DOWN:
			motionblur_properties.y -= 0.01f;
			break;
		case GLFW_KEY_ESCAPE:
		case GLFW_KEY_Q:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		}
	}
}

int main()
{
	content.set_clear_color(WHITE);
	content.set_eye_pos(glm::vec3(0,0,-3));
	content.run(draw_loop, init, key_callback);

	return 0;
}
