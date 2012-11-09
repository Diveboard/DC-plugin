#/**********************************************************\ 
#
# Auto-Generated Plugin Configuration file
# for DiveBoard
#
#\**********************************************************/

set(PLUGIN_NAME "DiveBoard")
set(PLUGIN_PREFIX "DBO")
set(COMPANY_NAME "DiveBoardcom")

# ActiveX constants:
set(FBTYPELIB_NAME DiveBoardLib)
set(FBTYPELIB_DESC "DiveBoard 1.0 Type Library")
set(IFBControl_DESC "DiveBoard Control Interface")
set(FBControl_DESC "DiveBoard Control Class")
set(IFBComJavascriptObject_DESC "DiveBoard IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "DiveBoard ComJavascriptObject Class")
set(IFBComEventSource_DESC "DiveBoard IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID 2d127d52-6923-55e3-93d7-c2a4678e0264)
set(IFBControl_GUID a4fc268d-ec46-52e7-964d-c17418183697)
set(FBControl_GUID bb65e3bf-46ae-5ee4-8278-d9e51551c499)
set(IFBComJavascriptObject_GUID aa4bdc2d-a3cb-5c63-9379-31b23d383b40)
set(FBComJavascriptObject_GUID 56e42fcb-eb88-5c6a-96cc-5c3bad18ed2e)
set(IFBComEventSource_GUID 3f348210-c944-573d-897a-3dfaa044af0d)
if ( FB_PLATFORM_ARCH_32 )
    set(FBControl_WixUpgradeCode_GUID 873540c2-d81e-56c2-8747-af27d08aaf3e)
else ( FB_PLATFORM_ARCH_32 )
    set(FBControl_WixUpgradeCode_GUID 28dffb94-03e5-560f-87c1-2ba3a3d667a8)
endif ( FB_PLATFORM_ARCH_32 )


# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "DiveBoardcom.DiveBoard")
set(MOZILLA_PLUGINID "diveboardcom.com/DiveBoard")

# strings
set(FBSTRING_CompanyName "DiveBoard")
set(FBSTRING_FileDescription "Dive computer plugin for DiveBoard.com")
set(FBSTRING_PluginDescription "Dive computer plugin for DiveBoard.com")
set(FBSTRING_PLUGIN_VERSION "1.0.0")
set(FBSTRING_LegalCopyright "Copyright 2010 DiveBoard")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}.dll")
set(FBSTRING_ProductName "DiveBoard")
set(FBSTRING_FileExtents "")
if ( FB_PLATFORM_ARCH_32 )
    set(FBSTRING_PluginName "DiveBoard")  # No 32bit postfix to maintain backward compatability.
else ( FB_PLATFORM_ARCH_32 )
    set(FBSTRING_PluginName "DiveBoard_${FB_PLATFORM_ARCH_NAME}")
endif ( FB_PLATFORM_ARCH_32 )

set(FBSTRING_MIMEType "application/x-diveboard")

# Mac plugin settings
set(FBMAC_USE_QUICKDRAW 0)
set(FBMAC_USE_CARBON 0)
set(FBMAC_USE_COCOA 0)
set(FBMAC_USE_COREGRAPHICS 0)
set(FBMAC_USE_COREANIMATION 0)
set(FBMAC_USE_INVALIDATINGCOREANIMATION 0)

set(FB_GUI_DISABLED 1)
#set(FB_ATLREG_MACHINEWIDE 1)
