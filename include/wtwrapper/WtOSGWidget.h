
#ifndef OSGVIEWER_WTOSGWIDGET_H
#define OSGVIEWER_WTOSGWIDGET_H

#include <wtwrapper/WtWrapper.h>

#pragma warning (push)
#pragma warning (disable: 4251) //("vector needs  dll-interface")
#pragma warning (disable: 4275) //("non dll-interface class used as base for dll-interface")
#pragma warning (disable: 4996) // use of non safe version of std::string copy by boost
#include <Wt/WGLWidget>
#include <Wt/WMatrix4x4>
#pragma warning (pop)

#include <osgViewer/GraphicsWindow>

#if defined (_WIN32) 
#if defined(wtwOSG_EXPORTS)
#define  WTWO_API __declspec(dllexport)
#else
#define  WTWO_API __declspec(dllimport)
#endif /* WTW_API */
#else /* defined (_WIN32) */
#define WTWO_API
#endif

class MyIncrementalCompileOperation;

namespace Wt
{
    class WCheckBox;
}

namespace osgViewer {
    class Viewer;
}

namespace osgUtil {
    class CullVisitor;
}

namespace osgWt
{

class WTWO_API WtOSGWidget : public Wt::WGLWidget
{
    typedef Wt::WGLWidget inherited;

public:

    WtOSGWidget (const unsigned int & width, const unsigned int & height, Wt::WContainerWidget *parent = NULL);
    virtual ~WtOSGWidget ();


    void setViewer(osg::ref_ptr<osgViewer::Viewer> viewer ){m_viewer = viewer;}


    // Specialization of WGLWidgeT::intializeGL()
    void initializeGL();

    // Specialization of WGLWidgeT::paintGL()
    void paintGL();

    // Specialization of WGLWidgeT::updateGL()
    void updateGL(void);

    // Specialization of WGLWidgeT::resizeGL()
    void resizeGL(int width, int height);


    void keyUp(const Wt::WKeyEvent& event);
    void keyDown(const Wt::WKeyEvent& event);

    // set the number of milliseconds the client waits before querying a new frame from the server
    void setFrameUpdateTimeout(const unsigned int & milliseconds){m_frameUpdateTimeOut = milliseconds;}

private:
    
    // Send a signal event to return the value of one particular matrix.
    void callbackMatrix(void);

    // Read back the values of the matrix.
    void readCallbackMatrix(std::string a);

    // Updates the client side matrix that is set as projection matrix
    void updateView(const unsigned int & width, const unsigned int &height);

    void defineJavaScript();

    void setClientSideLookAtHandler(const JavaScriptMatrix4x4 &m,
        double fromX, double fromY, double fromZ,
        double toX, double toY, double toZ,
        double upX, double upY, double upZ);

    // Signal to be sent
    Wt::JSignal<std::string> * jMVMatrixSignal_;

    // Matrix to hold the values.
    Wt::WMatrix4x4 mvMatrix;

    // milliseconds before querying a new update from the server
    unsigned int m_frameUpdateTimeOut;

protected:

    // override to load custom javascript (interaction)
    virtual void render(Wt::WFlags<Wt::RenderFlag> flags);

    osg::ref_ptr<osgViewer::Viewer> m_viewer;

    osg::ref_ptr<MyIncrementalCompileOperation> m_ico;

    Wt::WCheckBox* m_checkBox;
};
}

#endif // OSGVIEWER_WTOSGWIDGET_H
