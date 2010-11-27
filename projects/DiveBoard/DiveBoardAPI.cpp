/**********************************************************\

  Auto-generated DiveBoardAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"

#include "DiveBoardAPI.h"
#include "Logger.h"
#include "Computer.h"
#include "ComputerFactory.h"

#include <boost/thread.hpp>


///////////////////////////////////////////////////////////////////////////////
/// @fn DiveBoardAPI::DiveBoardAPI(DiveBoardPtr plugin, FB::BrowserHostPtr host)
///
/// @brief  Constructor for your JSAPI object.  You should register your methods, properties, and events
///         that should be accessible to Javascript from here.
///
/// @see FB::JSAPIAuto::registerMethod
/// @see FB::JSAPIAuto::registerProperty
/// @see FB::JSAPIAuto::registerEvent
///////////////////////////////////////////////////////////////////////////////
DiveBoardAPI::DiveBoardAPI(DiveBoardPtr plugin, FB::BrowserHostPtr host) : m_plugin(plugin), m_host(host)
{
	alwaysAsync=true;
	status.nbDivesRead=-1;
	status.nbDivesTotal=-1;
	status.state = COMPUTER_NOT_STARTED;
	comp = NULL;

	
	registerMethod("echo",      make_method(this, &DiveBoardAPI::echo));
    registerMethod("testEvent", make_method(this, &DiveBoardAPI::testEvent));

	registerMethod("extract", make_method(this, &DiveBoardAPI::extract));
    registerMethod("detect", make_method(this, &DiveBoardAPI::detect));

    // Read-only property
    registerProperty("logs",         make_property(this, &DiveBoardAPI::get_logs));
    registerProperty("nbDivesRead",  make_property(this, &DiveBoardAPI::get_nbDivesRead));
    registerProperty("nbDivesTotal", make_property(this, &DiveBoardAPI::get_nbDivesTotal));
    registerProperty("status",       make_property(this, &DiveBoardAPI::get_status));

	
	// Read-write property
    registerProperty("testString",
                     make_property(this,
                        &DiveBoardAPI::get_testString,
                        &DiveBoardAPI::set_testString));

    // Read-only property
    registerProperty("version",
                     make_property(this,
                        &DiveBoardAPI::get_version));
    
    
    registerEvent("onfired");    
    registerEvent("onloaded");    
}

///////////////////////////////////////////////////////////////////////////////
/// @fn DiveBoardAPI::~DiveBoardAPI()
///
/// @brief  Destructor.  Remember that this object will not be released until
///         the browser is done with it; this will almost definitely be after
///         the plugin is released.
///////////////////////////////////////////////////////////////////////////////
DiveBoardAPI::~DiveBoardAPI()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @fn DiveBoardPtr DiveBoardAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
DiveBoardPtr DiveBoardAPI::getPlugin()
{
    DiveBoardPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}



// Read/Write property testString
std::string DiveBoardAPI::get_testString()
{
    return m_testString;
}
void DiveBoardAPI::set_testString(const std::string& val)
{
    m_testString = val;
}

// Read-only property version
std::string DiveBoardAPI::get_version()
{
    return "CURRENT_VERSION";
}

// Read-only property version
std::string DiveBoardAPI::get_logs()
{
	return Logger::toString();
}

// Read-only property version
std::string DiveBoardAPI::get_status()
{
	ComputerStatus local;
	if (comp) local = comp->get_status();
	else local = status;

	if (local.state == COMPUTER_NOT_STARTED)
		return( "COMPUTER_NOT_STARTED" );
	else if (local.state == COMPUTER_RUNNING)
		return( "COMPUTER_RUNNING" );
	else if (local.state == COMPUTER_FINISHED)
		return( "COMPUTER_FINISHED" );

	return( "STATUS_UNKNOWN" );
}

// Read-only property version
int DiveBoardAPI::get_nbDivesRead()
{
	ComputerStatus local;
	if (comp) local = comp->get_status();
	else local = status;
	return local.nbDivesRead;
}

// Read-only property version
int DiveBoardAPI::get_nbDivesTotal()
{
	ComputerStatus local;
	if (comp) local = comp->get_status();
	else local = status;
	return local.nbDivesTotal;
}

// Method echo
FB::variant DiveBoardAPI::echo(const FB::variant& msg)
{
	Logger::append(str(boost::format("coucou %d") %1));
    return msg;
}

void DiveBoardAPI::testEvent(const FB::variant& var)
{
    FireEvent("onfired", FB::variant_list_of(var)(true)(1));
}



//todo mac
//DWORD WINAPI DiveBoardAPI::asyncfunc(LPVOID lpParam)
void *DiveBoardAPI::asyncfunc(void *p)
{
	DiveBoardAPI *plugin = (DiveBoardAPI*)p;

	std::string diveXML;

	Logger::append("Start async");

	if (plugin->status.state == COMPUTER_RUNNING) {
		Logger::append("get_all_dives is already running");
	} else if (!plugin->comp) {
		Logger::append("computer is null !");
	} else {
		plugin->status.state = COMPUTER_RUNNING;
		try{
			Logger::append("Running get_all_dives asynchronously");
			plugin->comp->get_all_dives(diveXML);
		} catch (...) {};

		plugin->status = plugin->comp->get_status();

		plugin->status.state = COMPUTER_FINISHED;
		delete plugin->comp;
		plugin->comp = NULL;
		
		
		//todo fix
		plugin->FireEvent("onloaded", FB::variant_list_of(FB::variant(diveXML)));
	}
	
	Logger::append("End async");
	
	return(NULL);
}


void DiveBoardAPI::extract(const std::string& port, const std::string& label)
{
	Logger::append("Extract called with device %s on port %s", label.c_str(), port.c_str());

		ComputerFactory factory;

		if (comp) {
			Logger::append("Computer already running ?");
			return;
		}

		comp = factory.createComputer(label, port);
		if (!comp) {
			Logger::append("No computer found");
			return;
		}

		if (alwaysAsync)
		{
			//TODO for mac
			//CreateThread(NULL, 0, &DiveBoardAPI::asyncfunc, this, 0, NULL);
			//boost::thread *th = new boost::thread( boost::bind(&DiveBoardAPI::asyncfunc, this));
			pthread_t threads;
			pthread_create(&threads, NULL, &DiveBoardAPI::asyncfunc, (void *) this);
		}
		else {
			std::string diveXML;
			comp->get_all_dives(diveXML);
			FireEvent("onloaded", FB::variant_list_of(FB::variant(diveXML)));
			delete comp;
			comp = NULL;
		}

		//todo: error catching
		//throw FB::script_error("conversion failed :(");
		return;
}


void DiveBoardAPI::detect()
{
		ComputerFactory factory;

		if (comp) {
			Logger::append("Computer already running ?");
			return;
		}

		comp = factory.detectConnectedDevice();
		if (!comp) {
			Logger::append("No computer found");
			return;
		}

		if (alwaysAsync)
		{
			//TODO for mac
			//CreateThread(NULL, 0, &DiveBoardAPI::asyncfunc, this, 0, NULL);
			boost::thread *th = new boost::thread( boost::bind(&DiveBoardAPI::asyncfunc, this)); 
		}
		else {
			std::string diveXML;
			comp->get_all_dives(diveXML);
			FireEvent("onloaded", FB::variant_list_of(FB::variant(diveXML)));
			delete comp;
			comp = NULL;
		}
}