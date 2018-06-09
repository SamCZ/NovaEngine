#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <map>

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "Engine/Render/Mesh/Mesh.h"
#include "Engine/Utils/File.h"

namespace NovaEngine {
	enum class ProgramType {
		Vertex,
		Geometry,
		Fragment,
		TessControl,
		TessEvaluation,
		Compute
	};

	typedef struct SHADER_SOURCE {
		ProgramType type;
		std::string program;
		File* fileSource;
	} ShaderSource;

	struct Attribute {
		GLuint id;
		std::string name;
	};

	struct SUniform {
		std::string name;
		GLenum type;
		bool isArray;
		int arrSize;
		float floatVal;
		int intVal;
		glm::vec2 vec2Val;
		glm::vec3 vec3Val;
		glm::vec4 vec4Val;
	};

	class Shader {
	public:
		Shader();
		~Shader();

		void deleteShader();

		bool compile();
		void addShader(ProgramType type, std::string program, File fileSource);

		void setUniform1i(std::string name, int v);
		void setUniform1f(std::string name, float v);
		void setUniform2f(std::string name, glm::vec2 vec);
		void setUniform3f(std::string name, glm::vec3 vec);
		void setUniform4f(std::string name, glm::vec4 vec);
		void setUniformMatrix4f(std::string name, glm::mat4 mat);

		void setUniform1iv(std::string name, const GLint* value);
		void setUniform1fv(std::string name, const GLfloat* value);
		void setUniform2fv(std::string name, const GLfloat* value);
		void setUniform3fv(std::string name, const GLfloat* value);
		void setUniform4fv(std::string name, const GLfloat* value);
		void setUniformMatrix3fv(std::string name, const GLfloat* value);
		void setUniformMatrix4fv(std::string name, const GLfloat* value);

		int getAttributeLocation(std::string alias);
		void bindAttributeLocation(GLuint location, std::string alias);

		void bindFragOutput(int attachement, std::string name);

		Attribute* getAttribute(MeshBuffer::Type type);

		SUniform* getSUniform(std::string name);
		std::vector<SUniform>& getSUniforms();

		GLuint getId() const;

		static std::string getShaderTypeExtensionName(ProgramType type);

		int getProgramCount();

		bool isCompiled() const;

		static std::vector<ProgramType> getAllProgramTypes();
	private:
		std::map<ProgramType, ShaderSource> _programs;
		std::map<std::string, GLint> _uniformLocationCahce;
		std::vector<GLuint> _programIds;
		std::map<MeshBuffer::Type, Attribute*> _attributes;
		std::vector<SUniform> _uniforms;
		GLuint _shaderProgram;
		bool _isCompiled;

		bool compileProgram(ProgramType type, std::string source);

		GLenum getShaderEnumType(ProgramType type);
		std::string getShaderName(ProgramType type);
		GLint getUniformLocation(std::string name);

		bool checkError(GLuint shader, GLuint flag, bool isProgram, ProgramType* type);

		bool getUniformArrayIndex(std::string name, std::string* nname, int* index);
	};
}

#endif // !SHADER_H