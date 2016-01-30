#include <sxge/opengl/shader.hh>

namespace sxge {

Shader::Shader(Shader::ShaderType type, std::string fileName) :
	_shaderType(type), _isCompiled(false)
{
	init(type);
	compileFromFile(fileName);
}

Shader::Shader(Shader::ShaderType type, const char *shaderSource)
{
	init(type);
	compileFromString(shaderSource);
}

Shader::~Shader()
{
	glDeleteShader(_shaderID);
}

Shader::ShaderType
Shader::shaderType() const
{
	return _shaderType;
}

GLuint
Shader::shaderID() const
{
	return _shaderID;
}

bool
Shader::isCompiled() const
{
	return _isCompiled;
}

//char *getSource() const;

void
Shader::init(Shader::ShaderType type)
{
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
		sxge_errs("failed to create shader");
	}
}

bool
Shader::compileFromString(const char *src)
{
	if (isCompiled()) {
		sxge_errs("shader already compiled");
		return false;
	}

	if (!src) {
		sxge_errs("shader source is NULL");
		return false;
	}

	GLint result;
	glShaderSource(_shaderID, 1, &src, NULL);
	glCompileShader(_shaderID);
	glGetShaderiv(_shaderID, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE) {
		sxge_err("failed to compile shader %x", _shaderID);
		logShaderError();
		glDeleteShader(_shaderID);
	}

	return false;
}

bool
Shader::compileFromFile(std::string filename)
{
	sxge::FileLoader loader(filename);
	return compileFromString(loader.getData());
}

void
Shader::logShaderError()
{
	GLint logLen;
	glGetShaderiv(_shaderID, GL_INFO_LOG_LENGTH, &logLen);
	if (!logLen) {
		return;
	}
	GLsizei realLogLen;
	char *log = new char[logLen];
	glGetShaderInfoLog(_shaderID, logLen, &realLogLen, log);
	sxge_err("shader %x log: %s", _shaderID, log);
	delete[] log;
}

} //namespace sxge
