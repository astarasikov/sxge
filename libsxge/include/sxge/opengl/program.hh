#ifndef __SXGE_OPENGL_PROGRAM_HH__
#define __SXGE_OPENGL_PROGRAM_HH__

#include <sxge/platform/opengl/gl_common.hh>
#include <sxge/opengl/shader.hh>
#include <list>
#include <algorithm>

namespace sxge {

class ShaderProgram {
public:
	typedef sxge::Shader::ShaderType ShaderType;
	typedef sxge::Shader Shader;

	ShaderProgram();
	virtual ~ShaderProgram();

	void addShader(sxge::Shader *shader);
	void addShaderFromSource(ShaderType type, const char* shaderCode);
	void addShaderFromFile(ShaderType type, std::string fileName);
	void removeShader(sxge::Shader *shader);

	bool link();
	bool isLinked() const;

	bool bind();
	void unbind();

	GLuint programID() const;
protected:
	typedef std::list<sxge::Shader*> shlist;

	GLuint _programID;
	bool _isLinked;
	shlist shaderList;

	void logProgramError();
};

} //namespace sxge

#endif //__SXGE_OPENGL_PROGRAM_HH__
