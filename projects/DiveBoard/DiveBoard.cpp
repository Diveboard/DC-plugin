/**********************************************************\

  Auto-generated DiveBoard.cpp

  This file contains the auto-generated main plugin object
  implementation for the DiveBoard project

\**********************************************************/

#include "NpapiTypes.h"
#include "DiveBoardAPI.h"

#include "DiveBoard.h"

///////////////////////////////////////////////////////////////////////////////
/// @fn DiveBoard::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginInitialize()
///
/// @see FB::FactoryBase::globalPluginInitialize
///////////////////////////////////////////////////////////////////////////////
void DiveBoard::StaticInitialize()
{
    // Place one-time initialization stuff here; note that there isn't an absolute guarantee that
    // this will only execute once per process, just a guarantee that it won't execute again until
    // after StaticDeinitialize is called
}

///////////////////////////////////////////////////////////////////////////////
/// @fn DiveBoard::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginDeinitialize()
///
/// @see FB::FactoryBase::globalPluginDeinitialize
///////////////////////////////////////////////////////////////////////////////
void DiveBoard::StaticDeinitialize()
{
    // Place one-time deinitialization stuff here
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  DiveBoard constructor.  Note that your API is not available
///         at this point, nor the window.  For best results wait to use
///         the JSAPI object until the onPluginReady method is called
///////////////////////////////////////////////////////////////////////////////
DiveBoard::DiveBoard()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  DiveBoard destructor.
///////////////////////////////////////////////////////////////////////////////
DiveBoard::~DiveBoard()
{
}

void DiveBoard::onPluginReady()
{
    // When this is called, the BrowserHost is attached, the JSAPI object is
    // created, and we are ready to interact with the page and such.  The
    // PluginWindow may or may not have already fire the AttachedEvent at
    // this point.
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Creates an instance of the JSAPI object that provides your main
///         Javascript interface.
///
/// Note that m_host is your BrowserHost and shared_ptr returns a
/// FB::PluginCorePtr, which can be used to provide a
/// boost::weak_ptr<DiveBoard> for your JSAPI class.
///
/// Be very careful where you hold a shared_ptr to your plugin class from,
/// as it could prevent your plugin class from getting destroyed properly.
///////////////////////////////////////////////////////////////////////////////
FB::JSAPIPtr DiveBoard::createJSAPI()
{
    // m_host is the BrowserHost
    return boost::make_shared<DiveBoardAPI>(FB::ptr_cast<DiveBoard>(shared_from_this()), m_host);
}

bool DiveBoard::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse down at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool DiveBoard::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse up at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool DiveBoard::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse move at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}
bool DiveBoard::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *w)
{
    // The window is attached; act appropriately
    return false;
}

bool DiveBoard::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
    // The window is about to be detached; act appropriately
    return false;
}

