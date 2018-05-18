#pragma once

#include "opengl.h"
#include "glm.h"
#include "vertex.h"
#include "load_image.h"
#include "varhandle.h"
#include <vector>

// This is a struct for a Mesh.
// Holds VBO, texture and entity properties.
struct Mesh
{
private:
	const char * TAG = "Entity";
public:

	GLuint
		vao,
		buffer,
		tex = GL_TEXTURE0;
	int
		data_size = 0;

	glm::vec3
		rotation = glm::vec3(0, 1, 0),
		pre_rotation = glm::vec3(0, 1, 0),
		scale = glm::vec3(1, 1, 1),
		pos, vel;
	GLfloat
		theta,
		pre_theta;

	Mesh() {}

	// Texture filename, Vertex data pack, world position, dynamic axis of rotation, and amount, static axis of rotation, and amount, scale vector. 
	Mesh(
		const char *texfilename,
		std::vector<Vertex>	data,
		glm::vec3 _pos,
		glm::vec3 _rotation,
		GLfloat _theta,
		glm::vec3 _pre_rotation,
		GLfloat _pre_theta,
		glm::vec3 _scale
	)
	{
		printf("[%-11s] New primitive object loaded: \n", TAG);
		printf("[%-11s]    Vertex count : %i\n", TAG, data.size());

		pos = _pos;
		rotation = _rotation;
		theta = _theta;
		scale = _scale;
		pre_rotation = _pre_rotation;
		pre_theta = _pre_theta;

		load_textures(texfilename);
		init(&data);
	}

	// Buffers Vertex data into the VBO
	void init(std::vector<Vertex> * d)
	{
		data_size = d->size();
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, data_size * sizeof(struct Vertex), d->data(), GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
			(const GLvoid*)offsetof(struct Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
			(const GLvoid*)offsetof(struct Vertex, color));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
			(const GLvoid*)offsetof(struct Vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer((GLuint)3, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
			(const GLvoid*)offsetof(struct Vertex, uv));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer((GLuint)4, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
			(const GLvoid*)offsetof(struct Vertex, tangent));
		glEnableVertexAttribArray(4);
		glBindVertexArray(0);
		printf("[%-11s]    Buffered VAO -> %i\n", TAG, vao);
		glFlush();
	}

	// Loads image file into a texture
	void load_textures(const char *texfilename)
	{
		if (texfilename != "")
		{
			tex = load_texture_from_image(texfilename);
			printf("[%-11s]    Texture file:   %s -> %i\n", TAG, texfilename, tex);
		}
		else
		{
			printf("[%-11s]    No texture loaded\n", TAG);

		}
	}

	// Draws the mesh including linking the model matrix
	void draw(int wire_frame,VarHandle *model,VarHandle *texture_handle)
	{
		model->load(get_model_mat());
		draw_array(wire_frame, texture_handle);
	}

	// Draws just the VBO and activating the texture
	void draw_array(int wire_frame, VarHandle *texture_handle)
	{
		// load the textures
		if (tex != GL_TEXTURE0)
		{
			load_texture_handle(texture_handle);
			glActiveTexture(GL_TEXTURE0 + tex);
			glBindTexture(GL_TEXTURE_2D, tex);
		}

		// draw the data
		glBindVertexArray(vao);
		glDrawArrays(wire_frame ? GL_LINE_LOOP : GL_TRIANGLES, 0, data_size);
		glBindVertexArray(0);

		// unload the texture
		if (tex != GL_TEXTURE0)
		{
			glActiveTexture(GL_TEXTURE0 + tex);
			glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);
		}
		glActiveTexture(GL_TEXTURE0);
		glFinish();
	}

	// Override the texture handle seperately
	void load_texture_handle(VarHandle * handle)
	{
		handle->load(tex);
	}

	// Sets the texture
	void set_tex(GLuint tex)
	{
		this->tex = tex;
	}

	// Get the model matrix
	glm::mat4 get_model_mat()
	{
		return glm::translate(glm::mat4(1.), pos) *
			glm::rotate(glm::mat4(1.), theta, rotation) *
			glm::rotate(glm::mat4(1.), pre_theta, pre_rotation) *
			glm::scale(glm::mat4(1.), scale);
	}
};