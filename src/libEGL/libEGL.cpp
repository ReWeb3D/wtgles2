//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// libEGL.cpp: Implements the exported EGL functions. This is just an 
// empty implementation as it is currently not needed by ReWeb3D


#include <EGL/egl.h>
#include <EGL/eglplatform.h>

#ifdef WIN32
#define LIBEGL_EXPORT EGLAPIENTRY
#else
#define LIBEGL_EXPORT 
#endif

extern "C"
{
 EGLint LIBEGL_EXPORT eglGetError(void){return 0;}

 EGLDisplay LIBEGL_EXPORT eglGetDisplay(EGLNativeDisplayType display_id){return 0;}

 EGLBoolean LIBEGL_EXPORT eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor){return true;}

 EGLBoolean LIBEGL_EXPORT eglTerminate(EGLDisplay dpy){return 0;}

 const char *LIBEGL_EXPORT eglQueryString(EGLDisplay dpy, EGLint name){return 0;}
 
 EGLBoolean LIBEGL_EXPORT eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config){return 0;}
 
 EGLBoolean LIBEGL_EXPORT eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config){return 0;}

 EGLBoolean LIBEGL_EXPORT eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value){return 0;}

 EGLSurface LIBEGL_EXPORT eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list){return 0;}

 EGLSurface LIBEGL_EXPORT eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list){return 0;}

 EGLSurface LIBEGL_EXPORT eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list){return 0;}

 EGLBoolean LIBEGL_EXPORT eglDestroySurface(EGLDisplay dpy, EGLSurface surface){return 0;}

 EGLBoolean LIBEGL_EXPORT eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value){return 0;}

 EGLBoolean LIBEGL_EXPORT eglQuerySurfacePointerANGLE(EGLDisplay dpy, EGLSurface surface, EGLint attribute, void **value){return 0;}

 EGLBoolean LIBEGL_EXPORT eglBindAPI(EGLenum api){return 0;}

 EGLenum LIBEGL_EXPORT eglQueryAPI(void){return 0;}

 EGLBoolean LIBEGL_EXPORT eglWaitClient(void){return 0;}

 EGLBoolean LIBEGL_EXPORT eglReleaseThread(void){return 0;}

 EGLSurface LIBEGL_EXPORT eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list){return 0;}

 EGLBoolean LIBEGL_EXPORT eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value){return 0;}

 EGLBoolean LIBEGL_EXPORT eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer){return 0;}

 EGLBoolean LIBEGL_EXPORT eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer){return 0;}

 EGLBoolean LIBEGL_EXPORT eglSwapInterval(EGLDisplay dpy, EGLint interval){return 0;}

 EGLContext LIBEGL_EXPORT eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list){return 0;}

 EGLBoolean LIBEGL_EXPORT eglDestroyContext(EGLDisplay dpy, EGLContext ctx){return 0;}

 EGLBoolean LIBEGL_EXPORT eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx){return 0;}

 EGLContext LIBEGL_EXPORT eglGetCurrentContext(void){return 0;}

 EGLSurface LIBEGL_EXPORT eglGetCurrentSurface(EGLint readdraw){return 0;}

 EGLDisplay LIBEGL_EXPORT eglGetCurrentDisplay(void){return 0;}

 EGLBoolean LIBEGL_EXPORT eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value){return 0;}

 EGLBoolean LIBEGL_EXPORT eglWaitGL(void){return 0;}

 EGLBoolean LIBEGL_EXPORT eglWaitNative(EGLint engine){return 0;}

 EGLBoolean LIBEGL_EXPORT eglSwapBuffers(EGLDisplay dpy, EGLSurface surface){return 0;}

 EGLBoolean LIBEGL_EXPORT eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target){return 0;}

 EGLBoolean LIBEGL_EXPORT eglPostSubBufferNV(EGLDisplay dpy, EGLSurface surface, EGLint x, EGLint y, EGLint width, EGLint height){return 0;}

 __eglMustCastToProperFunctionPointerType LIBEGL_EXPORT eglGetProcAddress(const char *procname){return 0;}

}
