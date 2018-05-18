#pragma once

#include <vector>
#include <set>
#include "mesh.h"

#include "load_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


struct MatInfo
{
public:
	glm::vec3 colour;
	GLuint textId; // HAS TO BE THE SAME AS TEXTURE_ID, the same thing that you pass in to partition vertices
	const char* name;
	const char* fileName;

	MatInfo(glm::vec3 c, GLuint t, const char* n, const char* fn)
	{
		colour = c;
		textId = t;
		name = n;
		fileName = fn;
	}
};

struct TexturedMesh : Mesh
{
private:
	std::vector<Mesh> meshes;

public:

	glm::vec3 pos;

	TexturedMesh(){}

	TexturedMesh(
		const char * obj_filename, 
		const char * base_filename,
		glm::vec3 _pos,
		glm::vec3 _rotation,
		GLfloat _theta,
		glm::vec3 _pre_rotation,
		GLfloat _pre_theta,
		glm::vec3 _scale)
	{
		//std::vector<tinyobj::shape_t> shapes;
		//std::vector<tinyobj::material_t> materials;

		//std::map<std::string, GLuint> texture_map;

		//std::string obj_err =
		//	tinyobj::LoadObj(shapes, materials, obj_filename, mtl_filename);

		//printf("[%-11s]	   ERRORs       : [ %s ]\n", "TEX_MODEL_LOAD", obj_err);


		//for (int i = 0; i < materials.size(); i++) {
		//	printf("material[%d].diffuse_texname = %s\n", i, materials[i].diffuse_texname.c_str());

		//	//Load texture

		//	std::string tex_name = materials[i].diffuse_texname;
		//	GLuint t = load_texture_from_image(tex_name.c_str());
		//	texture_map.insert(std::make_pair(tex_name.c_str(), t));
		//}

		//std::vector<glm::vec3> vertices;
		//std::vector<glm::vec3> normals;
		//std::vector<GLuint> texture_ids;
		//std::vector<glm::vec2> texture_coords;

		//// Go through every shape that makes up the model
		//for (int i = 0; i < shapes.size(); i++) {
		//	printf("adding positions (i,max_i) = (%d,%d)\n", i, shapes.size());

		//	// Go through every vertex
		//	for (int j = 0; j < shapes[i].mesh.indices.size(); j++) {
		//		printf("adding positions (j,max_j) = (%d,%d)\n", j, shapes[i].mesh.indices.size()); //   / 100.f
		//		vertices.push_back(glm::vec3(
		//			shapes[i].mesh.positions[shapes[i].mesh.indices[j] * 3],
		//			shapes[i].mesh.positions[shapes[i].mesh.indices[j] * 3 + 1],
		//			shapes[i].mesh.positions[shapes[i].mesh.indices[j] * 3 + 2]
		//		));

		//		normals.push_back(glm::vec3(
		//			shapes[i].mesh.normals[shapes[i].mesh.indices[j] * 3],
		//			shapes[i].mesh.normals[shapes[i].mesh.indices[j] * 3 + 1],
		//			shapes[i].mesh.normals[shapes[i].mesh.indices[j] * 3 + 2]
		//		));

		//		// If this is a textured vertex
		//		if (shapes[i].mesh.texcoords.size() != 0) {

		//			// Assign texture to triangle
		//			printf("adding text ids\n");
		//			int texture_index = shapes[i].mesh.material_ids[(int)floor((float)j / 3.f)];
		//			GLuint texture_index_offset = texture_map.find(materials[texture_index].diffuse_texname.c_str())->second;
		//			texture_ids.push_back(texture_index_offset);

		//			// Assign point of texture to sample
		//			printf("adding tex_coords (%d of %d)\n", shapes[i].mesh.indices[j] * 2 + 1, shapes[i].mesh.texcoords.size());
		//			texture_coords.push_back(glm::vec2(
		//				shapes[i].mesh.texcoords[shapes[i].mesh.indices[j] * 2],
		//				shapes[i].mesh.texcoords[shapes[i].mesh.indices[j] * 2 + 1]
		//			));

		//			//printf("material[%d].diffuse_texname = %s\n", i, materials[i].diffuse_texname);
		//		}
		//		else {
		//			// Assign texture to triangle
		//			printf("vertex is coloured, no texture\n");
		//			texture_ids.push_back(-1);
		//			texture_coords.push_back(glm::vec2(0.f, 0.f));
		//		}
		//	}
		//}
		std::vector<tinyobj::shape_t> shapes = std::vector<tinyobj::shape_t>();
		std::vector<tinyobj::material_t> materials = std::vector<tinyobj::material_t>();

		std::map<GLuint, MatInfo> materialMap = std::map<GLuint, MatInfo>();

		std::string obj_file = std::string(base_filename).append(obj_filename).append(".obj");
		std::string mtl_file = std::string(base_filename).append(obj_filename).append(".mtl");
		std::string base = std::string(base_filename);

		std::string obj_err =
			tinyobj::LoadObj(shapes, materials, obj_file.c_str(), base.c_str());

		// Go through each material, map its info, and load its texture
		for (int i = 0; i < materials.size(); i++) {
			std::string name = materials[i].name;
			float* colour = materials[i].diffuse;

			printf("diffuse tex name %s\n", materials[i].diffuse_texname.c_str());
			printf("name = %s\n", materials[i].name.c_str());

			// Load texture if one exists
			if (materials[i].diffuse_texname != "") {
				std::string tex_name = materials[i].diffuse_texname;
				tex_name = std::string(base_filename).append(tex_name.c_str()).c_str();
				GLuint textId = load_texture_from_image(tex_name.c_str());

				//texture_map.insert(std::make_pair(materials[i].diffuse_texname.c_str(), t));

				materialMap.insert(std::make_pair(i, MatInfo(glm::vec3(colour[0], colour[1], colour[2]), textId, name.c_str(), tex_name.c_str())));
			}
			else {
				materialMap.insert(std::make_pair(i, MatInfo(glm::vec3(colour[0], colour[1], colour[2]), 1, name.c_str(), "")));
			}


			//if (materialMap.find(i) == materialMap.end())
		}

		//materials.push_back(tinyobj::material_t());

		std::vector<glm::vec3> vertices = std::vector<glm::vec3>();
		std::vector<glm::vec3> colours = std::vector<glm::vec3>();
		std::vector<glm::vec3> normals = std::vector<glm::vec3>();
		std::vector<GLuint> texture_ids = std::vector<GLuint>();
		std::vector<glm::vec2> texture_coords = std::vector<glm::vec2>();

		// Go through every shape that makes up the model
		for (int i = 0; i < shapes.size(); i++) {
			//printf("adding positions (i,max_i) = (%d,%d)\n", i, shapes.size());

			// Go through every vertex
			for (int j = 0; j < shapes[i].mesh.indices.size(); j++) {
				//printf("adding positions (j,max_j) = (%d,%d)\n", j, shapes[i].mesh.indices.size()); //   / 100.f
				vertices.push_back(glm::vec3(
					shapes[i].mesh.positions[shapes[i].mesh.indices[j] * 3],
					shapes[i].mesh.positions[shapes[i].mesh.indices[j] * 3 + 1],
					shapes[i].mesh.positions[shapes[i].mesh.indices[j] * 3 + 2]
				));

				int texture_index = shapes[i].mesh.material_ids[(int)floor((float)j / 3.f)];
				//const char* texture_index_offset = materials[texture_index].diffuse_texname.c_str();

				//GLuint texture_index_offset = texture_map.find(materials[texture_index].diffuse_texname.c_str())->second;

				//colours.push_back();
				normals.push_back(glm::vec3(
					shapes[i].mesh.normals[shapes[i].mesh.indices[j] * 3],
					shapes[i].mesh.normals[shapes[i].mesh.indices[j] * 3 + 1],
					shapes[i].mesh.normals[shapes[i].mesh.indices[j] * 3 + 2]
				));
				// Assign texture to triangle
				//printf("adding text ids\n");
				texture_ids.push_back(texture_index);

				// If this is a textured vertex
				if (shapes[i].mesh.texcoords.size() != 0) {


					// Assign point of texture to sample
					//printf("adding tex_coords (%d of %d)\n", shapes[i].mesh.indices[j] * 2 + 1, shapes[i].mesh.texcoords.size());
					texture_coords.push_back(glm::vec2(
						shapes[i].mesh.texcoords[shapes[i].mesh.indices[j] * 2],
						shapes[i].mesh.texcoords[shapes[i].mesh.indices[j] * 2 + 1]
					));

					//printf("material[%d].diffuse_texname = %s\n", i, materials[i].diffuse_texname);
				}
				else {
					// Assign texture to triangle
					//printf("vertex is coloured, no texture\n");
					texture_coords.push_back(glm::vec2(1/(float)j, 1/(float)j));
				}
			}
		}

		std::set<GLuint> texture_id_set(texture_ids.begin(), texture_ids.end());
		std::vector<GLuint> unique_texture_ids;
		unique_texture_ids.assign(texture_id_set.begin(), texture_id_set.end());
		for (int i = 0; i < unique_texture_ids.size(); i++)
		{
			std::vector<Vertex> data;
			int index = unique_texture_ids.at(i);
			GLuint this_texture = materialMap.at(index).textId;
			//printf("%i", materialMap.at(index).textId)
			// Add every vertex that belong to a particular texture
			for (int j = 0; j < vertices.size(); j++) {
				if (texture_ids.at(j) == index) {
					Vertex vert;

					vert.position = vertices[j];
					vert.normal = normals[j];
					vert.color = materialMap.at(index).colour;
					vert.uv = texture_coords[j];
					
					data.push_back(vert);
				}
			}
			Mesh m = Mesh(
				"", 
				data, 
				glm::vec3(),
				glm::vec3(1,0,0), 0.0f, 
				glm::vec3(1,0,0), 0.0f, 				
				glm::vec3(1, 1, 1));
			m.set_tex(this_texture);
			meshes.push_back(m);
		}

		pos = _pos;
		rotation = _rotation;
		theta = _theta;
		scale = _scale;
		pre_rotation = _pre_rotation;
		pre_theta = _pre_theta;
	}

	virtual void draw(int wire_frame, VarHandle *model, VarHandle *texture_handle)
	{
		glm::mat4 c_model = get_model_mat();
		for (Mesh m : meshes)
		{
			model->load(c_model * m.get_model_mat());
			m.draw_array(wire_frame, texture_handle);
		}
	}

	void add_mesh(Mesh mesh)
	{
		meshes.push_back(mesh);
	}
};