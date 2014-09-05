#ifndef __FFMPEG_GL_CONTROLLER__H__
#define __FFMPEG_GL_CONTROLLER__H__

#import "sxge_view.h"

@interface GLController : NSWindow
-(void)createGLView;

@property(nonatomic, readwrite, retain) SxgeView *glView;
@end

#endif //__FFMPEG_GL_CONTROLLER__H__
