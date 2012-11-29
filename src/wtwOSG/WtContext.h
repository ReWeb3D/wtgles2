#ifndef WTOSG_WTCONTEXT
#define WTOSG_WTCONTEXT 1
// class to provide a GraphicsContext for osg


#include <osg/GraphicsContext>

namespace osgWt
{

class WtGraphicsContext : public osg::GraphicsContext 
{
public:

    WtGraphicsContext()
    {
        setState( new osg::State );
        getState()->setGraphicsContext(this);
    }

    bool valid(void) const
    {
        return true;
    }

    
    bool realizeImplementation(void)
    {
        return true;
    }
    
    bool isRealizedImplementation(void) const
    {
        return true;
    }
        
    void closeImplementation(void)
    {
    }

    bool makeCurrentImplementation(void)
    {
        return true;
    }
    
    bool makeContextCurrentImplementation(osg::GraphicsContext *)
    {   return true;}
    
    bool releaseContextImplementation(void)
    {
        return true;
    }
    
    void bindPBufferToTextureImplementation(GLenum)
    {}

    void swapBuffersImplementation(void)
    {}

};

struct WtWindowingSystemInterface : public osg::GraphicsContext::WindowingSystemInterface
{
    virtual unsigned int getNumScreens(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier = osg::GraphicsContext::ScreenIdentifier()) 
    {
        return 1;
    };

    virtual void getScreenSettings(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, osg::GraphicsContext::ScreenSettings & resolution)
    {
        resolution = osg::GraphicsContext::ScreenSettings();
    };

    virtual bool setScreenSettings(const osg::GraphicsContext::ScreenIdentifier& /*screenIdentifier*/, const osg::GraphicsContext::ScreenSettings & /*resolution*/) { return false; }

    virtual void enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, osg::GraphicsContext::ScreenSettingsList & resolutionList)
    {
    }

    virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits)
    {
        osg::GraphicsContext * gc = new WtGraphicsContext();

        return gc;
    }

    virtual ~WtWindowingSystemInterface() {}
};



//////////////////////////////////////////////////////////////////////////
// Hack to force initialization of the windowing system interface
struct WindowingSystemInterfaceInitializer
{
    WindowingSystemInterfaceInitializer()
    {
        osg::GraphicsContext::setWindowingSystemInterface(new WtWindowingSystemInterface());
    }
};

static WindowingSystemInterfaceInitializer s_wsiInitializer;
//////////////////////////////////////////////////////////////////////////
}

#endif // WTOSG_WTCONTEXT