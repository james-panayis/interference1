#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>


#include <cstdint>
#include <vector>
#include <iostream>

extern float projectionMatrix_[16];

namespace james {

	class image {

	public:

		image() {
			textureId_ = 0;
			vertexId_ = 0;

			if (!shaderCreated_) {
				create_shader();
			}
		};

		~image() {
			if (textureId_) {
				glDeleteTextures(1, &textureId_);
				textureId_ = 0;
			}

			if (vertexId_) {
				glDeleteBuffers(1, &vertexId_);
				vertexId_ = 0;
			}
		};

		void load(const std::uint8_t * data, int width, int height) {

			width_ = width;
			height_ = height;

			if (!textureId_) {

				// setup the texture

				if (data) {
					glGenTextures(1, &textureId_);

					glBindTexture(GL_TEXTURE_2D, textureId_);

					//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}

				// setup the vertex data

				std::vector<float> v;

				v.push_back(0.0);
				v.push_back(0.0);
				v.push_back(0.0);
				v.push_back(1.0);

				v.push_back(-2.0);
				v.push_back(-1.0);
				//v.push_back(0.0);
				//v.push_back(0.0);


				v.push_back(width);
				v.push_back(0.0);
				v.push_back(0.0);
				v.push_back(1.0);

				v.push_back(1.0);
				v.push_back(-1.0);
				//v.push_back(1.0);
				//v.push_back(0.0);


				v.push_back(0.0);
				v.push_back(height);
				v.push_back(0.0);
				v.push_back(1.0);

				v.push_back(-2.0);
				v.push_back(1.0);
				//v.push_back(0.0);
				//v.push_back(1.0);


				v.push_back(width);
				v.push_back(height);
				v.push_back(0.0);
				v.push_back(1.0);

				//v.push_back(1.0);
				//v.push_back(1.0);
				v.push_back(1.0);
				v.push_back(1.0);

				// store the vertex on the gpu

				glGenBuffers(1, &vertexId_);
				glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
				glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_DYNAMIC_DRAW);

			} else {
				if (data) glBindTexture(GL_TEXTURE_2D, textureId_);
			}

			if (data) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

				glBindTexture(GL_TEXTURE_2D, 0);
			}
		};


		void set_size(float new_width, float new_height) {
			width_ = new_width;
			height_ = new_height;
		};

		void update(float r1, float r2, float i1, float i2) {

			std::vector<float> v;

			v.push_back(0.0);
			v.push_back(0.0);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(r1);
			v.push_back(i1);

			v.push_back(width_);
			v.push_back(0.0);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(r2);
			v.push_back(i1);

			v.push_back(0.0);
			v.push_back(height_);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(r1);
			v.push_back(i2);

			v.push_back(width_);
			v.push_back(height_);
			v.push_back(0.0);
			v.push_back(1.0);

			v.push_back(r2);
			v.push_back(i2);

			// store the vertex on the gpu

			glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
			glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_DYNAMIC_DRAW);
		};


		void display(float xPos, float yPos, float iter1, float iter2, float iter3, float iter4) {

			glUseProgram(program_);

			glUniformMatrix4fv(uniformProj_, 1, GL_FALSE, projectionMatrix_);

			glUniform4f(uniformOffset_, xPos, yPos, 0.0, 0.0);
			glUniform2f(uniformScale_, 1.0, 1.0);
			glUniform4f(uniformRot_, 0.0, 0.0, 0.0, 0.0);
			glUniform4f(uniformIter_, iter1, iter2, iter3, iter4); //variable inputs

			glUniform1i(attribSample_, 0);

			glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
			glEnableVertexAttribArray(attribPosition_);
			glEnableVertexAttribArray(attribTexcoords_);

			glVertexAttribPointer(attribPosition_, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const GLvoid *)0);
			glVertexAttribPointer(attribTexcoords_, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const GLvoid *)(4 * sizeof(GLfloat)));

			glBindTexture(GL_TEXTURE_2D, textureId_);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glDisableVertexAttribArray(attribPosition_);
			glDisableVertexAttribArray(attribTexcoords_);
			glUseProgram(0);
		};

	private:

		bool create_shader()
		{
			GLint status;

			// build the vertex shader

			const char * vertexShaderProg[] = { "attribute vec4 position;\n",
				"attribute vec2 a_texCoord;\n",
				"uniform mat4 proj;\n",
				"uniform vec4 offset;\n",
				"uniform vec2 scale;\n",
				"uniform vec4 rot;\n",
				"varying vec4 v_texCoord;\n",
				"void main()\n",
				"{\n",
				"  mat4 a_scale = mat4(scale.x, 0.0, 0.0, 0.0, 0.0, scale.y, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);\n",
				"  mat4 rotation = mat4(cos(rot.z), sin(rot.z), 0, 0,  -sin(rot.z), cos(rot.z), 0, 0,  0, 0, 1, 0,  0, 0, 0, 1);\n",
				"  vec4 t = rotation * position;\n",
				"  gl_Position = proj * ((a_scale * t) + offset);\n",
				"  v_texCoord = vec4(a_texCoord.x, a_texCoord.y, 0.0, 1.0);\n",
				"}\n"
			};
			GLuint len = 15;

			vertexShader_ = glCreateShader(GL_VERTEX_SHADER);

			glShaderSource(vertexShader_, len, vertexShaderProg, NULL);
			glCompileShader(vertexShader_);

			glGetShaderiv(vertexShader_, GL_COMPILE_STATUS, &status);

			if (status == GL_FALSE) { return false; }

			// build the fragment shader
			
			const char * fragmentShaderProg[] = {
				"varying highp vec4 v_texCoord;\n", //range to calculate
				"uniform sampler2D s_texture;\n", //texture input
				"uniform highp vec4 iter;\n", //inputs
				"void main()\n",
				"{\n",
				"  highp float red = 0.0;\n", //these will be used in colouring (store values for colours)
				"  highp float green = 0.0;\n",
				"  highp float r = v_texCoord.x;\n", //coords to calculate over
				"  highp float i = v_texCoord.y;\n",
				"  highp float max_iteration = iter.x * 0.05;\n", //to depth
				"  highp float d1 = 0.0*sqrt(r*r +i*i)*iter.y/10.0;\n", //distance to each point
				"  highp float d2 = sqrt((r-1.0)*(r-1.0) + i*i)*(50.0);\n",//+0.9*iter.y/6.0)/2.;\n",
				"  highp float d3 = sqrt((r-iter.z)*(r-iter.z) + (i-iter[3])*(i-iter[3]))*50.0;\n",
				//"  highp float d1 = sqrt((r+iter.y/2000.0)*(r+iter.y/2000.0) +(i+iter.y/4000.0)*(i+iter.y/4000.0))*50.0;\n",
				//"  highp float d2 = sqrt((r-1.0)*(r-1.0) + i*i)*50.0;\n",
				//"  highp float d3 = sqrt((r-1.0)*(r-1.0) + (i-1.0)*(i-1.0))*50.0;\n",
				"  highp float c = (cos(d1)) + (cos(d2)) + (cos(d3));\n", //sum  vectors in 2d
				"  highp float s = (sin(d1)) + (sin(d2)) + (sin(d3));\n",
				//"  highp float c = cos(d1) + cos(d2);\n",
				//"  highp float s = sin(d1) + sin(d2);\n",
				"  highp float colour = (c*c + s*s)/4.0;\n", //calculate magnitude of resultant vector
																										 //"  highp float colour = (0.5 + 0.5*sqrt(c*c + s*s))/2.0;\n",
				"  gl_FragColor = vec4(2.0*(3.0*colour-2.0) - (3.0*colour-2.0)*(3.0*colour-2.0), 2.0*(2.0*colour-1.0) - (2.0*colour-1.0)*(2.0*colour-1.0), 2.0*colour - colour*colour, 1.0);\n", //colourise
																																																																																												 //"  gl_FragColor = vec4(4.0*colour - 4.0*colour*colour, 6.0*colour - 9.0*colour*colour, 2.0*colour - colour*colour, 1.0);\n",
																																																																																												 //"  gl_FragColor = vec4(colour*2.0-0.5, 0.75*colour+0.25, 2.0*colour - colour*colour, 1.0);\n",
																																																																																												 //"  gl_FragColor = vec4((sqrt((5.0 + log(colour)) * colour) * 120.0)/255.0, ((5.0 + log(colour)) * 35.0)/255.0, (colour*25.0)/255.0, 1.0);\n",
				"}\n"
			};

			len = 18;
			

			/*
			const char * fragmentShaderProg[] = { "varying vec4 v_texCoord;\n",
			"uniform sampler2D s_texture;\n",
			"void main()\n",
			"{\n",
			"  gl_FragColor = texture2DProj(s_texture, v_texCoord.xyw);\n",
			"}\n"
			};
			len = 6;
			*/
			fragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);

			glShaderSource(fragmentShader_, len, fragmentShaderProg, NULL);
			glCompileShader(fragmentShader_);

			glGetShaderiv(fragmentShader_, GL_COMPILE_STATUS, &status);

			if (status == GL_FALSE) { std::cout << "did not compile\n"; check_compiled(fragmentShader_); return false; }

			// link the program and store the entry points

			program_ = glCreateProgram();

			glAttachShader(program_, vertexShader_);
			glAttachShader(program_, fragmentShader_);

			glLinkProgram(program_);

			glGetProgramiv(program_, GL_LINK_STATUS, &status);

			if (status == GL_FALSE) { std::cout << "did not link\n"; return false; }

			attribPosition_ = glGetAttribLocation(program_, "position");
			attribTexcoords_ = glGetAttribLocation(program_, "a_texCoord");
			attribSample_ = glGetUniformLocation(program_, "s_texture");
			uniformProj_ = glGetUniformLocation(program_, "proj");
			uniformOffset_ = glGetUniformLocation(program_, "offset");
			uniformScale_ = glGetUniformLocation(program_, "scale");
			uniformRot_ = glGetUniformLocation(program_, "rot");
			uniformIter_ = glGetUniformLocation(program_, "iter");

			glUniform1i(attribSample_, 0); // try having this here rather than for each texture

			shaderCreated_ = true;

			return true;
		};

		static int check_compiled(GLuint shader) {

			GLint success = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

			if (success == GL_FALSE) {
				GLint max_len = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_len);

				GLchar err_log[max_len];
				glGetShaderInfoLog(shader, max_len, &max_len, &err_log[0]);
				glDeleteShader(shader);

				std::cout << "Shader compilation failed: " << err_log << std::endl;
			}

			return success;
		};


		static int check_linked(GLuint program) {

			GLint success = 0;
			glGetProgramiv(program, GL_LINK_STATUS, &success);

			if (success == GL_FALSE) {
				GLint max_len = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_len);

				GLchar err_log[max_len];
				glGetProgramInfoLog(program, max_len, &max_len, &err_log[0]);

				std::cout << "Program linking failed: " << err_log << std::endl;
			}

			return success;
		}


		// Store the opengl texture id

		GLuint textureId_;

		// Store the opengl vertex id

		GLuint vertexId_;

		float width_;
		float height_;

		// Shader setings

		static bool shaderCreated_;

		static GLuint program_;

		static GLuint vertexShader_;
		static GLuint fragmentShader_;

		static GLint  attribPosition_;
		static GLint  attribTexcoords_;
		static GLint  attribSample_;

		static GLint  uniformProj_;
		static GLint  uniformOffset_;
		static GLint  uniformScale_;
		static GLint  uniformRot_;
		static GLint  uniformIter_;

	}; // class image

	bool image::shaderCreated_ = false;

	GLuint image::program_ = 0;

	GLuint image::vertexShader_ = 0;
	GLuint image::fragmentShader_ = 0;

	GLint image::attribPosition_ = 0;
	GLint image::attribTexcoords_ = 0;
	GLint image::attribSample_ = 0;

	GLint image::uniformProj_ = 0;
	GLint image::uniformOffset_ = 0;
	GLint image::uniformScale_ = 0;
	GLint image::uniformRot_ = 0;
	GLint image::uniformIter_ = 0;

} // namespace
