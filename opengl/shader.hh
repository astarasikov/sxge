#ifndef __SXGE_OPENGL_SHADER_HH__
#define __SXGE_OPENGL_SHADER_HH__

#include "gl_common.hh"
#include "util/log.h"
#include "util/file_loader.hh"
#include <string>

namespace sxge {

class Shader {
public:
	enum ShaderType {
		Vertex = (1 << 0),
		Fragment = (1 << 1),
	};

	Shader(Shader::ShaderType type, std::string fileName) :
		_shaderType(type), _isCompiled(false) {
		init(type);
		compileFromFile(fileName);
	}

	Shader(Shader::ShaderType type, const char *shaderSource) {
		init(type);
		compileFromString(shaderSource);	
	}

	virtual ~Shader() {
		glDeleteShader(_shaderID);
	}
	
	Shader::ShaderType shaderType() const {
		return _shaderType;
	}

	GLuint shaderID() const {
		return _shaderID;
	}
	
	bool isCompiled() const {
		return _isCompiled;
	}

	char *getSource() const;
protected:
	enum ShaderType _shaderType;
	GLuint _shaderID;
	bool _isCompiled;

	void init(Shader::ShaderType type) {
		_shaderType = type;
		_isCompiled = false;
		
		GLenum glShaderType;
		switch (type) {
			case Shader::ShaderType::Vertex:
				glShaderType = GL_VERTEX_SHADER;
			break;
			case Shader::ShaderType::Fragment:
				glShaderType = GL_FRAGMENT_SHADER;
			break;
		}
		_shaderID = glCreateShader(glShaderType);
		if (!_shaderID) {
			err("failed to create shader");	
		}
	}
	
	bool compileFromString(const char *src) {
		if (isCompiled()) {
			err("shader already compiled");
			return false;
		}

		if (!src) {
			err("shader source is NULL");
			return false;
		}

		GLint result;
		glShaderSource(_shaderID, 1, &src, NULL);
		glCompileShader(_shaderID);		
		glGetShaderiv(_shaderID, GL_COMPILE_STATUS, &result);
		
		if (result == GL_FALSE) {
			err("failed to compile shader %x", _shaderID);
			logShaderError();
		}
		
		return false;
	}

	bool compileFromFile(std::string filename) {
		sxge::FileLoader loader(filename);
		return compileFromString(loader.getData());
	}

	void logShaderError() {
		GLint logLen;
		glGetShaderiv(_shaderID, GL_INFO_LOG_LENGTH, &logLen);
		if (!logLen) {
			return;
		}
		GLsizei realLogLen;
		char *log = new char[logLen];
		glGetShaderInfoLog(_shaderID, logLen, &realLogLen, log);
		err("shader %x log: %s\n", _shaderID, log);
		delete[] log;
	}
};

} //namespace sxge

#endif //__SXGE_OPENGL_SHADER_HH__
