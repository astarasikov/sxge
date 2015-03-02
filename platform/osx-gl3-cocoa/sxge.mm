#import <cassert>
#import <iostream>

#import <sxge/util/signals.h>
#import <sxge/apps/apps.hh>

#import "sxge_view.h"
#import "sxge_gl_controller.h"
#import "opengl_view.h"

@implementation SxgeView
{
	sxge::Screen *screen_;
}

-(void)prepareOpenGL
{
}

-(BOOL)acceptsFirstResponder {
	return YES;
}

-(void)mouseMoved:(NSEvent *)theEvent {
	NSPoint location = [theEvent locationInWindow];
	screen_->mouseEvent(location.x, location.y, sxge::Screen::MouseButton::NONE);
}

-(void)keyDown:(NSEvent *)theEvent {
	using namespace sxge;

	NSString *characters = [theEvent characters];
	if ([characters length] != 1) {
		return;
	}

	unichar rawKey = [characters characterAtIndex:0];
	unichar key = 0;

	NSUInteger flags = [theEvent modifierFlags];
	Screen::KeyStatus ks = Screen::KeyStatus::KS_Up;
	Screen::SpecialKey specialKey = Screen::SK_None;

	if (flags & NSControlKeyMask) {
		ks = (Screen::KeyStatus)(ks | Screen::KS_Ctrl);
	}
	if (flags & NSShiftKeyMask) {
		ks = (Screen::KeyStatus)(ks | Screen::KS_Shift);
	}
	if (flags & NSAlternateKeyMask) {
		ks = (Screen::KeyStatus)(ks | Screen::KS_Mod1);
	}
	if (flags & NSCommandKeyMask) {
		ks = (Screen::KeyStatus)(ks | Screen::KS_Mod4);
	}

	#define CASE(fun) \
		case NS ## fun ## FunctionKey: \
			specialKey = Screen::SK_ ## fun; \
			break;

	switch (rawKey) {
		case NSUpArrowFunctionKey:
			specialKey = Screen::SK_Up;
			break;
		case NSDownArrowFunctionKey:
			specialKey = Screen::SK_Down;
			break;
		case NSLeftArrowFunctionKey:
			specialKey = Screen::SK_Left;
			break;
		case NSRightArrowFunctionKey:
			specialKey = Screen::SK_Right;
			break;
		CASE(F1);
		CASE(F2);
		CASE(F3);
		CASE(F4);
		CASE(F5);
		CASE(F6);
		CASE(F7);
		CASE(F8);
		CASE(F9);
		CASE(F10);
		CASE(F11);
		CASE(F12);
		CASE(Delete);
		default:
			key = rawKey;
	}
	#undef CASE

	screen_->keyEvent(key, specialKey, ks);
}

-(void)renderForTime:(CVTimeStamp)time
{
	if ([self lockFocusIfCanDraw] == NO) {
		return;
	}
	id context = [self openGLContext];
	CGLContextObj contextObj = (CGLContextObj)[context CGLContextObj];
	if (kCGLNoError != CGLLockContext(contextObj)) {
		goto fail_cg_lock;
	}

	static int done = 0;
	if (!done) {
		screen_ = new sxge::Demo1_Cube();
		screen_->init();
		done = 1;
	}

	screen_->reshape(self.frame.size.width, self.frame.size.height);
	screen_->display();

	[context flushBuffer];

	CGLUnlockContext(contextObj);
fail_cg_lock:
	[self unlockFocus];
}
@end

int main(int argc, char** argv) {
	(void)sxge_setup_posix_signals();

	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	NSApplication *app = [NSApplication sharedApplication];
	GLController *controller = [[GLController alloc] init];
	//[NSApp setDelegate:controller];
	[app activateIgnoringOtherApps:YES];
	[app run];
	[pool release];
	return 0;
}
