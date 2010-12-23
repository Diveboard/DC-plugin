/**********************************************************\

  Auto-generated DiveBoardAPI.cpp

\**********************************************************/


#define _WIN32_WINNT 0x601
#define BOOST_LIB_DIAGNOSTIC


#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"

#include "DiveBoardAPI.h"
#include "Logger.h"
#include "Computer.h"
#include "ComputerFactory.h"

#include <boost/thread.hpp>

#include "DBException.h"





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
    registerProperty("name",         make_property(this, &DiveBoardAPI::get_name));

	
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
	registerEvent("onprogress"); 
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

std::string DiveBoardAPI::get_name()
{
    return "DiveBoard Reader";
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
    return str(boost::format("Version built on %s %s") % __DATE__ % __TIME__);
}

// Read-only property version
std::string DiveBoardAPI::get_logs()
{
	return Logger::toString();
}

// Read-only property version
FB::VariantMap DiveBoardAPI::get_status()
{
	ComputerStatus local;
	if (comp) local = comp->get_status();
	else local = status;

	FB::VariantMap ret;

	switch(local.state) 
	{
	case COMPUTER_NOT_STARTED:
		ret[std::string("state")] = FB::variant("COMPUTER_NOT_STARTED");
		break;
	case COMPUTER_RUNNING:
		ret[std::string("state")] = FB::variant("COMPUTER_RUNNING");
		break;
	case COMPUTER_FINISHED:
		ret[std::string("state")] = FB::variant("COMPUTER_FINISHED");
		break;
	default:
		ret[std::string("state")] = FB::variant("STATUS_UNKNOWN");
	}

	ret[std::string("nbDivesRead")] = FB::variant(local.nbDivesRead);
	ret[std::string("nbDivesTotal")] = FB::variant(local.nbDivesTotal);
	ret[std::string("percent")] = FB::variant(local.percent);

	return(ret);
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
	LOGINFO(str(boost::format("coucou %d") %1));
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
	std::string diveXML;
	DiveBoardAPI *plugin = (DiveBoardAPI*)p;

	LOGINFO("Start async");
	try{

		if (plugin->status.state == COMPUTER_RUNNING) throw DBException("DiveBoardAPI::asyncfunc : get_all_dives is already running");
		if (!plugin->comp) throw DBException("DiveBoardAPI::asyncfunc : computer is null !");
	
		plugin->status.state = COMPUTER_RUNNING;
		LOGINFO("Running get_all_dives asynchronously");

		plugin->comp->get_all_dives(diveXML);

		//Updating the status
		plugin->status = plugin->comp->get_status();
		plugin->status.state = COMPUTER_FINISHED;
		delete plugin->comp;
		plugin->comp = NULL;

		//todo fix
		plugin->FireEvent("onloaded", FB::variant_list_of(FB::variant(diveXML)));

		

	} catch (DBException e)
	{
		LOGINFO(std::string("ERROR :") + e.what());
		return(NULL);
	};
	LOGINFO("End async");
	
	return(NULL);
}


void DiveBoardAPI::extract(const std::string& strport, const std::string& label)
{
	try {
		LOGINFO("Extract called with device %s on port %s", label.c_str(), strport.c_str());
		std::string port;

#ifdef _WIN32
		port = strport;
#else
		port = strport;
#endif
		ComputerFactory factory;

		if (comp) throw DBException("Computer already running ?");

		comp = factory.createComputer(label, port);

		//todo : delete because errors should be handled through exceptions
		if (!comp) throw DBException("No computer found");

		if (alwaysAsync)
		{
			//CreateThread(NULL, 0, &DiveBoardAPI::asyncfunc, this, 0, NULL);
			boost::thread *th = new boost::thread( boost::bind(&DiveBoardAPI::asyncfunc, this));
			//pthread_t threads;
			//pthread_create(&threads, NULL, &DiveBoardAPI::asyncfunc, (void *) this);
		}
		else {
			std::string diveXML;
			comp->get_all_dives(diveXML);
			FireEvent("onloaded", FB::variant_list_of(FB::variant(diveXML)));
			delete comp;
			comp = NULL;
		}

	} catch(DBException e) 
	{
		//throw FB::script_error("conversion failed :(");
		LOGINFO(std::string("ERROR :") + e.what());
	}
}


FB::VariantMap DiveBoardAPI::detect()
//std::string DiveBoardAPI::detect()
{
	try
	{
		ComputerFactory factory;
		
		//if (comp) throw DBException("Computer already running ?");

		std::map<std::string, std::string> ret = factory.detectConnectedDevice();
		FB::VariantMap ret2;

	    for (std::map<std::string, std::string>::iterator it = ret.begin(); it != ret.end(); it++) {
			ret2[it->first] = FB::variant(it->second);
		}

		return(ret2);

	} catch(DBException e) 
	{
		//throw FB::script_error("conversion failed :(");
		LOGINFO(std::string("ERROR :") + e.what());
		throw(e);
	}
}