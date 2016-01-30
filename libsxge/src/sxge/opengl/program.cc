#include <sxge/opengl/program.hh>

namespace sxge {
ShaderProgram::ShaderProgram() : _isLinked(false) {
	_programID = glCreateProgram();
	if (!_programID) {
		sxge_errs("failed to create shader program");
	}
}

ShaderProgram::~ShaderProgram() {
	shaderList.clear();
}

void
ShaderProgram::addShader(sxge::Shader *shader)
{
	shaderList.push_back(shader);
}

void
ShaderProgram::addShaderFromSource(ShaderType type, const char* shaderCode)
{
	addShader(new sxge::Shader(type, shaderCode));
}

void
ShaderProgram::addShaderFromFile(ShaderType type, std::string fileName)
{
	addShader(new sxge::Shader(type, fileName));
}

void
ShaderProgram::removeShader(sxge::Shader *shader)
{
	shlist::iterator it;
	it = find(shaderList.begin(), shaderList.end(), shader);
	if (it != shaderList.end()) {
		shaderList.erase(it);
	}
}

bool
ShaderProgram::link()
{
	if (_isLinked) {
		sxge_errs("attempting to link an already linked program");
		return false;
	}

	for (auto shader : shaderList) {
		glAttachShader(_programID, shader->shaderID());
	}

	glLinkProgram(_programID);

	GLint status;
	glGetProgramiv(_programID, GL_LINK_STATUS, &status);
	if (GL_FALSE == status) {
		sxge_errs("failed to link shader program");
		logProgramError();
		return false;
	}

	_isLinked = true;
	return _isLinked;
}

bool
ShaderProgram::isLinked() const
{
	return _isLinked;
}

bool
ShaderProgram::bind()
{
	if (!isLinked()) {
		sxge_err("trying to bind an unlinked program %x", _programID);
		return false;
	}

	glUseProgram(_programID);
	return true;
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
}

GLuint
ShaderProgram::programID() const
{
	return _programID;
}

void
ShaderProgram::logProgramError()
{
	GLint logLen;
	GLsizei realLen;

	glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &logLen);
	if (!logLen) {
		sxge_err("no log for program %x", _programID);
		return;
	}

	char* log = new char[logLen];
	glGetProgramInfoLog(_programID, logLen, &realLen, log);
	sxge_err("program %x log: %s", _programID, log);
	delete[] log;
};

} //namespace sxge
