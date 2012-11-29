What is ReWeb3D ?
------------

ReWeb3D is a C++ framework to bring 3D rendering of OpenGLES2 
applications to web applications, using WebGL. It consists of an
adapted version of [Wt](http://www.webtoolkit.eu/wt) and the 
wtgles2 wrapper library.
It has been tested successfully with [OpenSceneGraph](http://www.openscenegraph.org)
and [osgEarth](http://osgearth.org) applications.


How does it work
------------
OpenGLES2 is a subset of OpenGL developed especially for mobile and embedded
devices. It does not provide "fixed functions" of OpenGL 2 and below, but
is very close to WebGL, an OpenGL API for web browsers (Javascript).

ReWeb3D provides an OpenGLES2 implementation that you can link your application
to. If you start the application, OpenGLES2 calls are captured an serialized
as WebGL calls in an HTML page which is served over HTTP. The browser
loads this page and the renders the content of your application.

Demos, examples
---------------

As examples, you can find basic OpenSceneGraph examples ported to ReWeb3D
in the repository. Running versions may be found at http://demos.vicomtech.org/

Building
--------
To build ReWeb3D you need [CMake](http://www.cmake.org/CMake) to create 
the project files for your platform.

- wtgles2 at https://github.com/ReWeb3D/wtgles2.git
- adapted version of Wt at https://github.com/ReWeb3D/wt.git

Optional but recommended to see the examples

- OpenSceneGraph at http://www.openscenegraph.org/svn/osg/OpenSceneGraph/trunk
- osgEarth at https://github.com/ReWeb3D/osgearth.git

Build sequence 

- build Wt (with WRaster support), see the [Wt documentation](http://www.webtoolkit.eu/wt/doc/reference/html)
- build wtgles2 will provide an OpenGLES2 library and headers
- build OpenSceneGraph using the provided OpenGLES2 library and headers see [osg documentation](http://www.openscenegraph.org/projects/osg/wiki/Community/OpenGL-ES)
- build osgEarth using the compiled OpenGLES2 and OSG versions
- build wtgles2 examples

License
--------
You may use the library under the [GNU General Public License] (http://www.gnu.org/copyleft/gpl.html),
just like Wt.
Providing a more permissive license is not possible because of the OpenGLES2 license and Wt's license.


Acknowlegements
--------
This work has been supported by [Vicomtech](http://www.vicomtech.org).
