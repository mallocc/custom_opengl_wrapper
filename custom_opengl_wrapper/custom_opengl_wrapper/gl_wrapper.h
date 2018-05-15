#pragma once

#include "opengl.h"
#include "glm.h"
#include <chrono>
#include <thread>


//Returns random float
inline float		randf()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

//void printV(glm::vec3 v)
//{
//	printf("x: %f - y: %f - z: %f\n", v.x, v.y, v.z);
//}

//Error callback  
static void		error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

static void		key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == 2)
	{
		switch (key)
		{

		case GLFW_KEY_ESCAPE:
		case GLFW_KEY_Q:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		}
	}
}

struct GLContent
{
private:
	glm::vec2
		window_size = glm::vec2(800,600);

	glm::vec3
		eye_pos = glm::vec3(0, 0, -5),
		eye_look_pos,
		up = glm::vec3(0, 1, 0);

	glm::mat4
		model,
		view,
		projection;

	float 
		fov = 45.0f, 
		aspect_ratio = window_size.x / window_size.y, 
		near_z = 0.1f, 
		far_z = 100.0f;
	

	glm::mat4 getOrtho()
	{
		return glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 10.0f);
	}
	glm::mat4 getOrthoView()
	{
		return glm::lookAt(glm::vec3(-0.866, 0, 0), glm::vec3(), glm::vec3(0, 1, 0));
	}
	glm::mat4 getPerspective()
	{
		return glm::perspective(glm::radians(45.0f), (float)window_size.x / (float)window_size.y, 0.1f, 100.0f);;
	}
	glm::mat4 getPerspectiveView()
	{
		return glm::lookAt(eye_pos, eye_look_pos, up);
	}


	//GL graphics loop
	void			glLoop(void(*graphics_loop)(), GLFWwindow * window)
	{
		printf("\n");
		printf("Running GL loop...\n");

		//Main Loop  
		do
		{
			// start clock for this tick
			auto start = std::chrono::high_resolution_clock::now();

			graphics_loop();

			//Swap buffers  
			glfwSwapBuffers(window);
			//Get and organize events, like keyboard and mouse input, window resizing, etc...  
			glfwPollEvents();

			// stop clock
			auto finish = std::chrono::high_resolution_clock::now();
			int ms = float(std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count());
			long newWait = 5 - ms;// -(gm.gameSpeed);
			newWait = newWait < 0 ? 0 : newWait;
			// throttle the graphics loop to cap at a certain fps
			std::this_thread::sleep_for(std::chrono::milliseconds(newWait));

		} //Check if the ESC or Q key had been pressed or if the window had been closed  
		while (!glfwWindowShouldClose(window));

		printf("\n");
		printf("Window has closed. Application will now exit.\n");

		//Close OpenGL window and terminate GLFW  
		glfwDestroyWindow(window);
		//Finalize and clean up GLFW  
		glfwTerminate();

		exit(EXIT_SUCCESS);
	}
	//GL window initialise
	GLFWwindow *				initWindow(void(*init)())
	{
		GLFWwindow * window;

		//Set the error callback  
		glfwSetErrorCallback(error_callback);

		printf("Initialising GLFW...\n");
		//Initialize GLFW  
		if (!glfwInit())
		{
			exit(EXIT_FAILURE);
		}
#ifdef __APPLE__
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

		printf("Creating window...\n");
		//Create a window and create its OpenGL context  
		window = glfwCreateWindow(window_size.x, window_size.y, "Test Window", NULL, NULL);
		//If the window couldn't be created  
		if (!window)
		{
			fprintf(stderr, "Failed to open GLFW window.\n");
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		printf("Setting window context...\n");
		//This function makes the context of the specified window current on the calling thread.   
		glfwMakeContextCurrent(window);

		//Sets the key callback  
		glfwSetKeyCallback(window, key_callback);

		printf("Initialising GLEW...\n");
		//Initialize GLEW  
		GLenum err = glewInit();
		//If GLEW hasn't initialized  
		if (err != GLEW_OK)
		{
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);
		// Cull triangles which normal is not towards the camera
		glEnable(GL_CULL_FACE);
		// enable texturineg
		glEnable(GL_TEXTURE_2D);


		// init
		init();

		return window;
	}

public:

	GLContent() {}

	GLContent(glm::vec3 window_size, glm::vec3 eye_pos, glm::vec3 eye_look_pos, glm::vec3 up, float fov, float aspect_ratio, float near_z, float far_z)
	{
		this->window_size = window_size;
		this->eye_pos = eye_pos;
		this->eye_look_pos = eye_look_pos;
		this->up = up;
		this->fov = fov;
		this->aspect_ratio = aspect_ratio;
		this->near_z = near_z;
		this->far_z = far_z;
	}

	
	void run(void(*graphics_loop)(), void(*init)())
	{
		glLoop(graphics_loop, initWindow(init));
	}

	void loadPerspective()
	{
		view = getPerspectiveView();
		projection = getPerspective();
	}

	void loadOrtho()
	{
		view = getOrthoView();
		projection = getOrtho();
	}

	glm::vec3 * get_eye_pos()
	{
		return &eye_pos;
	}

	glm::mat4 * get_model_mat()
	{
		return &model;
	}
	glm::mat4 * get_view_mat()
	{
		return &view;
	}
	glm::mat4 * get_proj_mat()
	{
		return &projection;
	}

};
