/* -*-c++-*- OpenSceneGraph - Copyright (C) 2009 Wang Rui
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef OSGVIEWER_WTOSGWIDGET_H
#define OSGVIEWER_WTOSGWIDGET_H

//#include <QtCore/QMutex>
//#include <QtCore/QEvent>
//#include <QtCore/QQueue>
//#include <QtCore/QSet>
//#include <QtOpenGL/QGLWidget>

#include <Wt/WGLWidget>

#include <osgViewer/GraphicsWindow>

// TODO: use for decoration
#define OSGWT_EXPORT


namespace osgViewer {
    class Viewer;
}

namespace osgWt
{

class OSGWT_EXPORT WtOSGWidget : public Wt::WGLWidget
{
    typedef Wt::WGLWidget inherited;

public:

    WtOSGWidget (Wt::WContainerWidget *parent = NULL);
    //GLWidget( QWidget* parent = NULL, const QGLWidget* shareWidget = NULL, Qt::WindowFlags f = 0, bool forwardKeyEvents = false );
    //GLWidget( QGLContext* context, QWidget* parent = NULL, const QGLWidget* shareWidget = NULL, Qt::WindowFlags f = 0, bool forwardKeyEvents = false );
    //GLWidget( const QGLFormat& format, QWidget* parent = NULL, const QGLWidget* shareWidget = NULL, Qt::WindowFlags f = 0, bool forwardKeyEvents = false );
    virtual ~WtOSGWidget ();


    void setViewer(osg::ref_ptr<osgViewer::Viewer> viewer ){m_viewer = viewer;}


    // Specialization of WGLWidgeT::intializeGL()
    void initializeGL();

    // Specialization of WGLWidgeT::paintGL()
    void paintGL();

    // Specialization of WGLWidgeT::resizeGL()
    void resizeGL(int width, int height);

protected:
    osg::ref_ptr<osgViewer::Viewer> m_viewer;
};
}

#endif // OSGVIEWER_WTOSGWIDGET_H
