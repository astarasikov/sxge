#ifndef __SXGE_VIEW__H__
#define __SXGE_VIEW__H__

#import "opengl_view.h"

@interface SxgeView : MyOpenGLViewBase
-(void)renderForTime:(CVTimeStamp)time;
-(void)keyDown:(NSEvent*)theEvent;
-(void)mouseMoved:(NSEvent*)theEvent;
-(BOOL)acceptsFirstResponder;
@end

#endif //__SXGE_VIEW__H__
