#ifndef __SXGE_OPENGL_PROGRAM_HH__
#define __SXGE_OPENGL_PROGRAM_HH__

#include "opengl/gl_common.hh"
#include "opengl/shader.hh"
#include <list>
#include <algorithm>

namespace sxge {

class ShaderProgram {
public:
	typedef sxge::Shader::ShaderType ShaderType;
	typedef sxge::Shader Shader;

	ShaderProgram() : _isLinked(false) {
		_programID = glCreateProgram();
		if (!_programID) {
			err("failed to create shader program");
		}
	}

	virtual ~ShaderProgram() {
		shaderList.clear();
	}

	void addShader(sxge::Shader *shader) {
		shaderList.push_back(shader);
	}

	void addShaderFromSource(ShaderType type, const char* shaderCode) {
		addShader(new sxge::Shader(type, shaderCode));
	}

	void addShaderFromFile(ShaderType type, std::string fileName) {
		addShader(new sxge::Shader(type, fileName));
	}

	void removeShader(sxge::Shader *shader) {
		shlist::iterator it;
		it = find(shaderList.begin(), shaderList.end(), shader);
		if (it != shaderList.end()) {
			shaderList.erase(it);
		}
	}
	
	bool link() {
		if (_isLinked) {
			err("attempting to link an already linked program");
			return false;
		}
		
		for (auto shader : shaderList) {
			glAttachShader(_programID, shader->shaderID());
		}

		glLinkProgram(_programID);

		GLint status;
		glGetProgramiv(_programID, GL_LINK_STATUS, &status);
		if (GL_FALSE == status) {
			err("failed to link shader program");
			logProgramError();
			return false;
		}

		_isLinked = true;
		return _isLinked;
	}

	bool isLinked() const {
		return _isLinked;
	}

	bool bind() {
		if (!isLinked()) {
			err("trying to bind an unlinked program %x", _programID);
			return false;
		}

		glUseProgram(_programID);
		return true;
	}

	void unbind() {
		glUseProgram(0);
	}

	GLuint programID() const {
		return _programID;
	}
protected:
	typedef std::list<sxge::Shader*> shlist;

	GLuint _programID;
	bool _isLinked;
	shlist shaderList;

	void logProgramError() {
		GLint logLen;
		GLsizei realLen;
		
		glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &logLen);
		if (!logLen) {
			err("no log for program %x", _programID);
			return;
		}

		char* log = new char[logLen];
		glGetProgramInfoLog(_programID, logLen, &realLen, log);
		err("program %x log: %s", _programID, log);
		delete[] log;
	}
};

} //namespace sxge

#endif //__SXGE_OPENGL_PROGRAM_HH__
