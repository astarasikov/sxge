#ifndef __SXGE_OPENGL_SHADER_HH__
#define __SXGE_OPENGL_SHADER_HH__

#include <sxge/platform/opengl/gl_common.hh>
#include <sxge/util/log.h>
#include <sxge/util/file_loader.hh>
#include <string>

namespace sxge {

class Shader {
public:
	enum ShaderType {
		Vertex = (1 << 0),
		Fragment = (1 << 1),
	};

	Shader(Shader::ShaderType type, std::string fileName);
	Shader(Shader::ShaderType type, const char *shaderSource);

	virtual ~Shader();
	Shader::ShaderType shaderType() const;
	GLuint shaderID() const;
	bool isCompiled() const;

	char *getSource() const;
protected:
	enum ShaderType _shaderType;
	GLuint _shaderID;
	bool _isCompiled;

	void init(Shader::ShaderType type);
	bool compileFromString(const char *src);
	bool compileFromFile(std::string filename);
	void logShaderError();
};

} //namespace sxge

#endif //__SXGE_OPENGL_SHADER_HH__
