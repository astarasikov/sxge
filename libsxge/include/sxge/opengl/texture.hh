#ifndef __SXGE_OPENGL_TEXTURE_HH__
#define __SXGE_OPENGL_TEXTURE_HH__

#include <string>
#include <sxge/platform/opengl/gl_common.hh>

namespace sxge {

class Texture {
public:
	Texture(unsigned width, unsigned height, char *raw_data);
	Texture(std::string filename, unsigned width, unsigned height);
	virtual ~Texture();
	GLuint getTextureID() const;
	bool buffer(GLenum texture);
protected:
	GLuint textureID;
	bool isLoaded;

	unsigned width;
	unsigned height;

	bool init(char *raw_data);
};

} //namespace sxge

#endif //__SXGE_OPENGL_TEXTURE_HH__
