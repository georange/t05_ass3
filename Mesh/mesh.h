#pragma once
#include "common/icg_common.h"
#include "OpenGP/Eigen/Image.h"
#include "../noise.h"

#define STBI_ASSERT(x) 
#define STBI_MALLOC
#define STBI_REALLOC
#define STBI_FREE

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Mesh{
protected:
    GLuint _vao; ///< vertex array object
    GLuint _pid; ///< GLSL shader program ID
    //GLuint _tex_day; ///< Texture ID
    //GLuint _tex_night; ///< Texture ID
    //GLuint _tex;
	GLuint _height_tex;
	GLuint _diffuse_tex;

	GLuint _water;
	GLuint _sand;
	GLuint _grass;
	GLuint _rock;
	GLuint _snow;

    const std::string filename = "Mesh/grid.obj";
    OpenGP::SurfaceMesh mesh;
    GLuint _vpoint;    ///< memory buffer
    GLuint _vnormal;   ///< memory buffer

public:
    GLuint getProgramID(){ return _pid; }

	GLuint loadTexture(const char* path) {
		GLuint textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
		if (data) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(data);
		} else {
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}

    void init(){
        ///--- Compile the shaders
        _pid = OpenGP::load_shaders("Mesh/Mesh_vshader.glsl", "Mesh/Mesh_fshader.glsl");
        if(!_pid) exit(EXIT_FAILURE);
        check_error_gl();
        
		bool success = mesh.read(filename.c_str());
        assert(success);
        
		mesh.triangulate();
        mesh.update_vertex_normals();
        printf("Loaded mesh '%s' (#V=%d, #F=%d)\n", filename.c_str(), mesh.n_vertices(), mesh.n_faces());

        ///--- Vertex one vertex Array
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        check_error_gl();

        ///--- Vertex Buffer
        OpenGP::SurfaceMesh::Vertex_property<OpenGP::Point> vpoints = mesh.get_vertex_property<OpenGP::Point>("v:point");
        glGenBuffers(ONE, &_vpoint);
        glBindBuffer(GL_ARRAY_BUFFER, _vpoint);
        glBufferData(GL_ARRAY_BUFFER, mesh.n_vertices() * sizeof(vec3), vpoints.data(), GL_STATIC_DRAW);
        check_error_gl();

        ///--- Normal Buffer
        OpenGP::SurfaceMesh::Vertex_property<OpenGP::Normal> vnormals = mesh.get_vertex_property<OpenGP::Normal>("v:normal");
        glGenBuffers(ONE, &_vnormal);
        glBindBuffer(GL_ARRAY_BUFFER, _vnormal);
        glBufferData(GL_ARRAY_BUFFER, mesh.n_vertices() * sizeof(vec3), vnormals.data(), GL_STATIC_DRAW);
        check_error_gl();

        ///--- Index Buffer
        std::vector<unsigned int> indices;
        for(OpenGP::SurfaceMesh::Face_iterator fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit) {
            unsigned int n = mesh.valence(*fit);
            OpenGP::SurfaceMesh::Vertex_around_face_circulator vit = mesh.vertices(*fit);
            for(unsigned int v = 0; v < n; ++v) {
                indices.push_back((*vit).idx());
                ++vit;
            }
        }

        GLuint _vbo_indices;
        glGenBuffers(ONE, &_vbo_indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        check_error_gl();

        glUseProgram(_pid);

		_height_tex = generate_noise();

		GLuint tex_id = glGetUniformLocation(_pid, "height_map");
		check_error_gl();
		glUniform1i(tex_id, 0);
		check_error_gl();

		//int width, height, nrChannels;
		//unsigned char *data = stbi_load("Mesh/grass.tga", &width, &height, &nrChannels, 0);

		//glGenTextures(1, &_diffuse_tex);
		//glBindTexture(GL_TEXTURE_2D, _diffuse_tex);

		//check_error_gl();
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		//check_error_gl();

		////glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);
		//stbi_image_free(data);

		/*tex_id = glGetUniformLocation(_pid, "diffuse_map");
		glUniform1i(tex_id, 1);
		glUseProgram(0);*/

		//vector<char*> textures{ "Mesh/water.tga", "Mesh/sand.tga", "Mesh/grass.tga", "Mesh/rock.tga", "Mesh/snow.tga" };
		//vector<char*> name{ "water", "sand", "grass", "rock", "snow" };
		
		_water = loadTexture("Mesh/water.tga");
		tex_id = glGetUniformLocation(_pid, "water");
		glUniform1i(tex_id, 1);
		
		_sand = loadTexture("Mesh/sand.tga");
		tex_id = glGetUniformLocation(_pid, "sand");
		glUniform1i(tex_id, 2);

		_grass = loadTexture("Mesh/grass.tga");
		tex_id = glGetUniformLocation(_pid, "grass");
		glUniform1i(tex_id, 3);

		_rock = loadTexture("Mesh/rock.tga");
		tex_id = glGetUniformLocation(_pid, "rock");
		glUniform1i(tex_id, 4);

		_snow = loadTexture("Mesh/snow.tga");
		tex_id = glGetUniformLocation(_pid, "snow");
		glUniform1i(tex_id, 5);
		check_error_gl();

		glUseProgram(0);

//        //--- Load texture
//                glGenTextures(1, &_tex);
//                glBindTexture(GL_TEXTURE_2D, _tex);

//                std::vector<unsigned char> buffer, image;
//                loadFile(buffer, "Mesh/mrt.png");
//                unsigned long w, h;
//                decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());

//                check_error_gl();
//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//check_error_gl();
//                glTexImage2D(GL_TEXTURE_2D, /*level*/ 0, GL_RGBA8,
//                             w, h, 0,
//                             GL_RGBA, GL_UNSIGNED_BYTE, image.data());
//check_error_gl();
//                GLuint tex_id = glGetUniformLocation(_pid, "tex");
//                std::cout << tex_id << std::endl;
//                check_error_gl();
//                glUniform1i(tex_id, GL_TEXTURE0);

//                check_error_gl();
//                glUseProgram(0);

    }

	void draw(mat4& Model, mat4& View, mat4& Projection) {
		glUseProgram(_pid);
		glBindVertexArray(_vao);
		check_error_gl();

		float t = glfwGetTime();
		glUniform1f(glGetUniformLocation(_pid, "time"), t);

		///--- Vertex Attribute ID for Positions
		GLint vpoint_id = glGetAttribLocation(_pid, "vpoint");
		glEnableVertexAttribArray(vpoint_id);

		glBindBuffer(GL_ARRAY_BUFFER, _vpoint);
		glVertexAttribPointer(vpoint_id, 3 /*vec3*/, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _height_tex);

		/*glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _diffuse_tex);*/

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _water);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _sand);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, _grass);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, _rock);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, _snow);
		//check_error_gl();

		///--- Vertex Attribute ID for Normals
		//GLint vnormal_id = glGetAttribLocation(_pid, "vnormal");
		//glEnableVertexAttribArray(vnormal_id);
		//glBindBuffer(GL_ARRAY_BUFFER, _vnormal);
		//glVertexAttribPointer(vnormal_id, 3 /*vec3*/, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, _tex);

		///--- Set the MVP to vshader
		glUniformMatrix4fv(glGetUniformLocation(_pid, "MODEL"), 1, GL_FALSE, Model.data());
		glUniformMatrix4fv(glGetUniformLocation(_pid, "VIEW"), 1, GL_FALSE, View.data());
		glUniformMatrix4fv(glGetUniformLocation(_pid, "PROJ"), 1, GL_FALSE, Projection.data());

		check_error_gl();
		///--- Draw
		glDrawElements(GL_TRIANGLES, /*#vertices*/ 3 * mesh.n_faces(), GL_UNSIGNED_INT, ZERO_BUFFER_OFFSET);
		check_error_gl();

		///--- Clean up
		glBindVertexArray(0);
		glUseProgram(0);
	}
};
