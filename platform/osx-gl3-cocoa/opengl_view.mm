#import "opengl_view.h"

static CVReturn displayCallback(CVDisplayLinkRef displayLink,
	const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime,
	CVOptionFlags flagsIn, CVOptionFlags *flagsOut,
	void *displayLinkContext)
{
	MyOpenGLViewBase *view = (MyOpenGLViewBase*)displayLinkContext;
	[view renderForTime: *inOutputTime];
	return kCVReturnSuccess;
}

@implementation MyOpenGLViewBase
{
	CVDisplayLinkRef displayLink;
}

-(id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat*)format
{
	self = [super initWithFrame:frameRect pixelFormat:format];
	[self registerDisplayLink];
	[self setWantsBestResolutionOpenGLSurface:YES];
	return self;
}

-(void)registerDisplayLink
{
	CGDirectDisplayID displayID = CGMainDisplayID();
	CVReturn error = CVDisplayLinkCreateWithCGDisplay(displayID, &displayLink);
	NSAssert((kCVReturnSuccess == error),
		@"Creating Display Link error %d", error);

	error = CVDisplayLinkSetOutputCallback(displayLink, displayCallback, self);
	NSAssert((kCVReturnSuccess == error),
		@"Setting Display Link callback error %d", error);
	CVDisplayLinkStart(displayLink);
}

-(void)renderForTime:(CVTimeStamp)time
{
	NSLog(@"MyOpenGLViewBase: renderForTime");
}

-(void)initializeContext
{
	NSLog(@"MyOpenGLViewBase: initializeContext");
}

-(BOOL)acceptsFirstResponder {
	NSLog(@"MyOpenGLViewBase: acceptsFirstResponder");
	return YES;
}

-(void)keyDown:(NSEvent *)theEvent {
	NSLog(@"MyOpenGLViewBase: Key Event %@", theEvent);
}

-(void)mouseMoved:(NSEvent *)theEvent {
	NSLog(@"MyOpenGLViewBase: Mouse Event %@", theEvent);
}

-(void)dealloc
{
	[super dealloc];
}

-(void)windowWillClose:(NSNotification *)note {
	//CVDisplayLinkRelease(displayLink);
	[[NSApplication sharedApplication] terminate:self];
}
@end
