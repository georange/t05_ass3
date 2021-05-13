#pragma once
#include "common/icg_common.h"
#include "OpenGP/Eigen/Image.h"

#define STBI_ASSERT(x) 
#define STBI_MALLOC
#define STBI_REALLOC
#define STBI_FREE

//#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

class Skybox{
protected:
	GLuint skyboxVAO, skyboxVBO, cubemapTexture, _pid;

public:
	GLuint loadCubemap(vector<std::string> faces) {
		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++) {
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			check_error_gl();
			if (data) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
				check_error_gl();
			}
			else {
				std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return textureID;
	}

	void init() {
        ///--- Compile the shaders
        _pid = OpenGP::load_shaders("Skybox/s_vshader.glsl", "Skybox/s_fshader.glsl");
        if(!_pid) exit(EXIT_FAILURE);       
        glUseProgram(_pid);
     
        ///--- Vertex coordinates
        {
			const GLfloat skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f };
		 
            ///--- Buffer
			glGenVertexArrays(1, &skyboxVAO);
			glGenBuffers(1, &skyboxVBO);
			glBindVertexArray(skyboxVAO);
			glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			check_error_gl();
	
        }
		vector<std::string> faces {"Skybox/right.tga", "Skybox/left.tga", "Skybox/top.tga", "Skybox/bottom.tga", "Skybox/front.tga", "Skybox/back.tga" };
		cubemapTexture = loadCubemap(faces);
		check_error_gl();
        
 		GLuint tex_id = glGetUniformLocation(_pid, "skybox");
		glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
		check_error_gl();

		///--- to avoid the current object being polluted
		glBindVertexArray(0);
		glUseProgram(0);

    }
       
    void cleanup(){
        glBindVertexArray(0);
        glUseProgram(0);
       // glDeleteProgram(_pid);
		glDeleteVertexArrays(1, &skyboxVAO);
		glDeleteBuffers(1, &skyboxVAO);
    }
    
	void draw(mat4& Model, mat4& View, mat4& Projection, const mat4& M) {
		//glDepthMask(GL_FALSE);
		glUseProgram(_pid);
    
		// draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		//View = mat4(mat3(View));
		View(0, 3) = 0;
		View(1, 3) = 0;
		View(2, 3) = 0;
		View(3, 3) = 1;

		View(3, 0) = 0;
		View(3, 1) = 0;
		View(3, 2) = 0;

		glUniformMatrix4fv(glGetUniformLocation(_pid, "VIEW"), 1, GL_FALSE, View.data());
		glUniformMatrix4fv(glGetUniformLocation(_pid, "PROJ"), 1, GL_FALSE, Projection.data());
		GLuint M_id = glGetUniformLocation(_pid, "M");
		glUniformMatrix4fv(M_id, 1, false, M.data());
		
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
		//glDepthMask(GL_TRUE);

		glDepthFunc(GL_LESS); // set depth function back to default
		glBindVertexArray(0);        
        glUseProgram(0);
    }
};
