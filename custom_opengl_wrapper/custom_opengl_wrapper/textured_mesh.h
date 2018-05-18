#pragma once

#include <vector>
#include <set>
#include "mesh.h"

#include "load_image.h"

#include "primitive_generators.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


struct TexturedMesh : Mesh
{
private:
	std::vector<Mesh> meshes;

public:

	glm::vec3 pos;

	TexturedMesh(){}

	TexturedMesh(
		const char * obj_filename, 
		const char * mtl_filename,
		glm::vec3 _pos,
		glm::vec3 _rotation,
		GLfloat _theta,
		glm::vec3 _pre_rotation,
		GLfloat _pre_theta,
		glm::vec3 _scale)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::map<std::string, GLuint> texture_map;

		std::string obj_err =
			tinyobj::LoadObj(shapes, materials, obj_filename, mtl_filename);

		printf("[%-11s]	   ERRORs       : [ %s ]\n", "TEX_MODEL_LOAD", obj_err);


		for (int i = 0; i < materials.size(); i++) {
			printf("material[%d].diffuse_texname = %s\n", i, materials[i].diffuse_texname.c_str());

			//Load texture

			std::string tex_name = materials[i].diffuse_texname;
			GLuint t = load_texture_from_image(obj_filename);
			texture_map.insert(std::make_pair(materials[i].diffuse_texname.c_str(), t));
		}

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<GLuint> texture_ids;
		std::vector<glm::vec2> texture_coords;

		// Go through every shape that makes up the model
		for (int i = 0; i < shapes.size(); i++) {
			printf("adding positions (i,max_i) = (%d,%d)\n", i, shapes.size());

			// Go through every vertex
			for (int j = 0; j < shapes[i].mesh.indices.size(); j++) {
				printf("adding positions (j,max_j) = (%d,%d)\n", j, shapes[i].mesh.indices.size()); //   / 100.f
				vertices.push_back(glm::vec3(
					shapes[i].mesh.positions[shapes[i].mesh.indices[j] * 3],
					shapes[i].mesh.positions[shapes[i].mesh.indices[j] * 3 + 1],
					shapes[i].mesh.positions[shapes[i].mesh.indices[j] * 3 + 2]
				));

				normals.push_back(glm::vec3(
					shapes[i].mesh.normals[shapes[i].mesh.indices[j] * 3],
					shapes[i].mesh.normals[shapes[i].mesh.indices[j] * 3 + 1],
					shapes[i].mesh.normals[shapes[i].mesh.indices[j] * 3 + 2]
				));

				// If this is a textured vertex
				if (shapes[i].mesh.texcoords.size() != 0) {

					// Assign texture to triangle
					printf("adding text ids\n");
					int texture_index = shapes[i].mesh.material_ids[(int)floor((float)j / 3.f)];
					GLuint texture_index_offset = texture_map.find(materials[texture_index].diffuse_texname.c_str())->second;
					texture_ids.push_back(texture_index_offset);

					// Assign point of texture to sample
					printf("adding tex_coords (%d of %d)\n", shapes[i].mesh.indices[j] * 2 + 1, shapes[i].mesh.texcoords.size());
					texture_coords.push_back(glm::vec2(
						shapes[i].mesh.texcoords[shapes[i].mesh.indices[j] * 2],
						shapes[i].mesh.texcoords[shapes[i].mesh.indices[j] * 2 + 1]
					));

					//printf("material[%d].diffuse_texname = %s\n", i, materials[i].diffuse_texname);
				}
				else {
					// Assign texture to triangle
					printf("vertex is coloured, no texture\n");
					texture_ids.push_back(-1);
					texture_coords.push_back(glm::vec2(0.f, 0.f));
				}
			}
		}

		std::set<GLuint> texture_id_set(texture_ids.begin(), texture_ids.end());
		std::vector<GLuint> unique_texture_ids;
		unique_texture_ids.assign(texture_id_set.begin(), texture_id_set.end());
		for (int i = 0; i < unique_texture_ids.size(); i++)
		{
			std::vector<Vertex> data;
			GLuint this_texture = unique_texture_ids.at(i);
			// Add every vertex that belong to a particular texture
			for (int i = 0; i < vertices.size(); i++) {
				if (texture_ids.at(i) == this_texture) {
					Vertex vert;

					vert.position = vertices[i];
					vert.normal = normals[i];
					vert.color = glm::vec3(1.0f,0.0f,1.0f);
					vert.uv = texture_coords[i];

					data.push_back(vert);
				}
			}
			Mesh m = Mesh("", data, glm::vec3(), glm::vec3(), 0.0f, glm::vec3(), 0.0f, glm::vec3(1, 1, 1));
			m.set_tex(this_texture);
			meshes.push_back(m);
		}

	}

	void draw(VarHandle * model_handle, VarHandle * texture_handle)
	{
		glm::mat4 c_model = glm::translate(glm::mat4(1.0f), pos);
		for (Mesh m : meshes)
		{
			model_handle->load(c_model * m.get_model_mat());  
			m.draw_array(0, texture_handle);
		}
	}
};
