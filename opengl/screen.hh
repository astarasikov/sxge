#ifndef __SXGE_OPENGL_SCREEN_HH__
#define __SXGE_OPENGL_SCREEN_HH__

namespace sxge {

class Screen {
public:
	typedef enum MouseButton {
		NONE = 0,
		LEFT = (1 << 0),
		RIGHT = (1 << 1),
		MIDDLE = (1 << 2),
	} MouseButton;

	virtual ~Screen();
	virtual void keyEvent(char key, bool isDown) = 0;
	virtual void mouseEvent(unsigned x, unsigned y, MouseButton buttons) = 0;
	virtual void display(void) = 0;
	virtual void reshape(unsigned width, unsigned height) = 0;
};

} //namespace sxge

#endif //__SXGE_OPENGL_SCREEN_HH__
