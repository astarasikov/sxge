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

	typedef enum SpecialKey {
		SK_None = 0,
		SK_F1,
		SK_F2,
		SK_F3,
		SK_F4,
		SK_F5,
		SK_F6,
		SK_F7,
		SK_F8,
		SK_F9,
		SK_F10,
		SK_F11,
		SK_F12,

		SK_Left,
		SK_Up,
		SK_Down,
		SK_Right,

		SK_BackSpace,
		SK_Tab,
		SK_Return,
		SK_Delete,
		SK_Escape,
		SK_Space,
	} SpecialKey;

	typedef enum KeyStatus {
		KS_Up = 0,
		KS_Down = 1 << 0,
		KS_Shift = 1 << 1,
		KS_Ctrl = 1 << 2,
		KS_Mod1 = 1 << 3,
		KS_Mod2 = 1 << 4,
		KS_Mod3 = 1 << 5,
		KS_Mod4 = 1 << 6,
	} KeyStatus;

	virtual ~Screen() {}
	virtual void init(void) = 0;
	virtual void keyEvent(char key, SpecialKey sk, KeyStatus ks) = 0;
	virtual void mouseEvent(float x, float y, MouseButton buttons) = 0;
	virtual void display(void) = 0;
	virtual void reshape(unsigned width, unsigned height) = 0;
};

} //namespace sxge

#endif //__SXGE_OPENGL_SCREEN_HH__
