#pragma once

#include "opengl.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <map>

#include "varhandle.h"

typedef GLuint GLSLProgramID;

struct GLSLProgram
{
private:
	GLSLProgramID ID;

	GLuint VertexShaderID;
	GLuint FragmentShaderID;

	const char * vertex_file_path;
	const char * fragment_file_path;

	std::map<VarHandleID, VarHandle> handles;

	VarHandleID 
		model_mat,
		tex;

	const char * TAG = "GLSLProgram";

public:
	//Loads shaders from their files into a shader program (from opengl-tutorials.org)
	GLSLProgram() {};

	GLSLProgram(
		const char * vertex_file_path,
		const char * fragment_file_path		
	) {

		printf("[%-11s] Creating Shader Program:\n", TAG);

		this->vertex_file_path = vertex_file_path;
		this->fragment_file_path = fragment_file_path;

		// Create the shaders
		VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		// Read the Vertex Shader code from the file
		std::string VertexShaderCode;
		std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
		if (VertexShaderStream.is_open()) {
			std::string Line = "";
			while (getline(VertexShaderStream, Line))
				VertexShaderCode += "\n" + Line;
			VertexShaderStream.close();
		}
		else {
			printf("[%-11s]    Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", TAG, vertex_file_path);
			getchar();
		}

		// Read the Fragment Shader code from the file
		std::string FragmentShaderCode;
		std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
		if (FragmentShaderStream.is_open()) {
			std::string Line = "";
			while (getline(FragmentShaderStream, Line))
				FragmentShaderCode += "\n" + Line;
			FragmentShaderStream.close();
		}

		GLint Result = GL_FALSE;
		int InfoLogLength;


		// Compile Vertex Shader
		printf("[%-11s]    Compiling shader : %s\n", TAG, vertex_file_path);
		char const * VertexSourcePointer = VertexShaderCode.c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
		glCompileShader(VertexShaderID);

		// Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
			printf("[%-11s]    %s\n", TAG, &VertexShaderErrorMessage[0]);
		}



		// Compile Fragment Shader
		printf("[%-11s]    Compiling shader : %s\n", TAG, fragment_file_path);
		char const * FragmentSourcePointer = FragmentShaderCode.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
		glCompileShader(FragmentShaderID);

		// Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
			glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			printf("[%-11s]    %s\n", TAG, &FragmentShaderErrorMessage[0]);
		}



		// Link the program
		printf("[%-11s]    Linking program\n", TAG);
		GLuint ProgramID = glCreateProgram();
		glAttachShader(ProgramID, VertexShaderID);
		glAttachShader(ProgramID, FragmentShaderID);
		glLinkProgram(ProgramID);

		// Check the program
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("[%-11s]    %s\n", TAG, &ProgramErrorMessage[0]);
		}


		glDetachShader(ProgramID, VertexShaderID);
		glDetachShader(ProgramID, FragmentShaderID);

		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);

		this->ID = ProgramID;

		printf("[%-11s]    Loaded -> GLSL PROG: %i\n", TAG, ProgramID);
	}

	GLSLProgram * add_handle(VarHandle handle)
	{
		handle.init(ID);
		handles.insert({ handle.get_handle_id(), handle });
		return this;
	}

	void load()
	{
		glUseProgram(ID);
		for (auto& sm_pair : handles)
			sm_pair.second.load();
	}

	VarHandle * get_handle(VarHandleID id)
	{
		return &handles[id];
	}

	GLSLProgramID get_id()
	{
		return ID;
	}

	GLSLProgram * set_model_mat4_handle(glm::mat4 * mat)
	{
		VarHandle handle = VarHandle("u_m", mat);
		handle.init(ID);
		model_mat = handle.get_handle_id();
		add_handle(handle);		
		return this;
	}

	GLSLProgram * set_view_mat4_handle(glm::mat4 * mat)
	{
		return add_handle(VarHandle("u_v", mat));
	}

	GLSLProgram * set_proj_mat4_handle(glm::mat4 * mat)
	{
		return add_handle(VarHandle("u_p", mat));
	}

	GLSLProgram * set_tex_handle()
	{
		VarHandle handle = VarHandle("u_tex");
		handle.init(ID);
		tex = handle.get_handle_id();
		add_handle(handle);
		return this;
	}

	VarHandle * get_model_mat4_handle()
	{
		return get_handle(model_mat);
	}

	VarHandle * get_tex_handle()
	{
		return get_handle(tex);
	}

};

struct GLSLProgramManager
{
private:
	std::map<GLSLProgramID, GLSLProgram> shader_programs;
	GLSLProgramID current_program;

public:

	GLSLProgramManager() {};

	GLSLProgram * load_program(GLSLProgramID id)
	{		
		shader_programs[id].load();
		current_program = id;
		return get_program(id);
	}

	GLSLProgram * add_program(
		const char * vertex_file_path,
		const char * fragment_file_path
	)
	{
		GLSLProgram program = GLSLProgram(vertex_file_path, fragment_file_path);
		shader_programs.insert({ program.get_id(), program });
		return get_program(program.get_id());
	}

	GLSLProgram * add_program(GLSLProgram program)
	{
		
		shader_programs.insert({ program.get_id(), program });
		return get_program(program.get_id());
	}

	GLSLProgram * get_program(GLSLProgramID id)
	{
		return &shader_programs[id];
	}

	GLSLProgram * get_current_program()
	{
		return &shader_programs[current_program];
	}

	GLSLProgramID add_program(
		const char * vertex_file_path,
		const char * fragment_file_path,
		glm::mat4 * model_data,
		glm::mat4 * view_data,
		glm::mat4 * proj_data
	)
	{
		GLSLProgram program = GLSLProgram(vertex_file_path, fragment_file_path);
		program.set_model_mat4_handle(model_data);
		program.set_view_mat4_handle(view_data);
		program.set_proj_mat4_handle(proj_data);
		shader_programs.insert({ program.get_id(), program });
		return program.get_id();
	}
};