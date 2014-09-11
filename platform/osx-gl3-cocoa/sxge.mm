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

-(void)keyDown:(NSEvent *)theEvent {
	NSString *characters = [theEvent characters];
	if ([characters length] != 1) {
		return;
	}
	screen_->keyEvent([characters characterAtIndex:0],
		sxge::Screen::SK_None, sxge::Screen::KS_Down);
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
	[NSApp setDelegate:controller];
	[app activateIgnoringOtherApps:YES];
	[app run];
	[pool release];
	return 0;
}
