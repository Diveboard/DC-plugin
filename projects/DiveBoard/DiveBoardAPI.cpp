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


#define catchall() catch (std::exception &e)                 \
	{                                                        \
		LOGERROR("Caught Exception : %s", e.what());         \
		throw FB::script_error(e.what());                    \
	} catch(...)                                             \
	{                                                        \
		LOGERROR("Caught Exception Unknown");                \
		throw FB::script_error("Caught Exception Unknown");  \
	}





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
#if defined(__MACH__) || defined(__linux__)
		alwaysAsync=true;
#else
		alwaysAsync=true;
#endif
	
		LOGINFO(get_version());

		status.nbDivesRead=-1;
		status.nbDivesTotal=-1;
		status.state = COMPUTER_NOT_STARTED;
		comp = NULL;

		//methods
		registerMethod("echo",    make_method(this, &DiveBoardAPI::echo));
		registerMethod("fireEvent",    make_method(this, &DiveBoardAPI::fireEvent));
		registerMethod("dump", make_method(this, &DiveBoardAPI::dump));
		registerMethod("extract", make_method(this, &DiveBoardAPI::extract));
		registerMethod("detect",  make_method(this, &DiveBoardAPI::detect));
		registerMethod("allports",make_method(this, &DiveBoardAPI::allports));
		registerMethod("isComputerPluggedin",make_method(this, &DiveBoardAPI::isComputerPluggedin));
		registerMethod("setLogLevel", make_method(this, &DiveBoardAPI::setLogLevel));
		registerMethod("setLogSize", make_method(this, &DiveBoardAPI::setLogSize));
		registerMethod("cancel",  make_method(this, &DiveBoardAPI::cancel));

		// Read-only property
		registerProperty("name",         make_property(this, &DiveBoardAPI::get_name));
		registerProperty("nodeName",     make_property(this, &DiveBoardAPI::get_nodename));
		registerProperty("version",      make_property(this, &DiveBoardAPI::get_version));
		registerProperty("logs",         make_property(this, &DiveBoardAPI::get_logs));
		registerProperty("status",       make_property(this, &DiveBoardAPI::get_status));

		//Events    
		registerEvent("onfired");    
		registerEvent("onloaded");    
		registerEvent("onerror");    
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
    try {
		DiveBoardPtr plugin(m_plugin.lock());
		if (!plugin) {
			throw FB::script_error("The plugin is invalid");
		}
		return plugin;
	} catchall()
}

std::string DiveBoardAPI::get_name()
{
    try {
		return "DiveBoard Reader";
	} catchall()
}

std::string DiveBoardAPI::get_nodename()
{
    try {
		return "OBJECT";
	} catchall()
}


// Read-only property version
std::string DiveBoardAPI::get_version()
{
    try {
		return str(boost::format("Version built on %s %s") % __DATE__ % __TIME__);
	} catchall()
}

// Read-only property logs
std::string DiveBoardAPI::get_logs()
{
	try {
		return Logger::toString();
	} catchall()
}

// Read-only property status
FB::VariantMap DiveBoardAPI::get_status()
{
	try	{
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
	} catchall()
}

// Method echo
FB::variant DiveBoardAPI::echo(const FB::variant& msg)
{
	try	{
		LOGINFO("Method echo called");
		return msg;
	} catchall()
}

// Method fireEvent
FB::variant DiveBoardAPI::fireEvent(const FB::variant& msg)
{
	try	{
		LOGINFO("Method fireEvent called");
		this->FireEvent("onfired", FB::variant_list_of(FB::variant(msg)));
		return true;
	} catchall()
}



void *DiveBoardAPI::asyncfunc(void *p)
{
	LOGINFO("Start asynchronous treatment");
	std::string diveXML;
	DiveBoardAPI *plugin = (DiveBoardAPI*)p;

	try{
		//todo : better use mutex
		if (plugin->status.state == COMPUTER_RUNNING) DBthrowError("Only one instance of DiveBoard can access to the Computer at the same time");
		if (!plugin->comp) DBthrowError("computer is null");
	
		//Updating the status when starting the computer
		plugin->status.state = COMPUTER_RUNNING;
		LOGDEBUG("Running get_all_dives");
		
		plugin->comp->get_all_dives(diveXML);

		LOGDEBUG("get_all_dives finished");

		//Updating the status after it's finished
		plugin->status = plugin->comp->get_status();
		plugin->status.state = COMPUTER_FINISHED;
		delete plugin->comp;
		plugin->comp = NULL;

		LOGDEBUG("triggering onloaded event");

		//todo fix
		plugin->FireEvent("onloaded", FB::variant_list_of(FB::variant(diveXML)));

	} catch (std::exception &e)
	{
		LOGERROR("Caught Exception : %s", e.what());
		plugin->FireEvent("onerror", FB::variant_list_of(FB::variant(e.what())));
		plugin->status = plugin->comp->get_status();
		plugin->status.state = COMPUTER_FINISHED;
		delete plugin->comp;
		plugin->comp = NULL;
	} catch(...)
	{
		LOGERROR("Caught Exception Unknown");
		plugin->FireEvent("onerror", FB::variant_list_of(FB::variant("Undefined exception")));
		plugin->status = plugin->comp->get_status();
		plugin->status.state = COMPUTER_FINISHED;
		delete plugin->comp;
		plugin->comp = NULL;
	}

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
			asyncfunc((void*)this);
		}

	} catchall()
}


void *DiveBoardAPI::dump_async(void *p)
{
	LOGINFO("Start asynchronous treatment for dump");
	std::string buffer;
	DiveBoardAPI *plugin = (DiveBoardAPI*)p;

	try{
		//todo : better use mutex
		if (plugin->status.state == COMPUTER_RUNNING) DBthrowError("Only one instance of DiveBoard can access to the Computer at the same time");
		if (!plugin->comp) DBthrowError("computer is null");
	
		//Updating the status when starting the computer
		plugin->status.state = COMPUTER_RUNNING;
		LOGDEBUG("Running dump on computer");
		
		//plugin->comp->dump(buffer);

		LOGDEBUG("dump on computer finished");

		//Updating the status after it's finished
		plugin->status = plugin->comp->get_status();
		plugin->status.state = COMPUTER_FINISHED;
		delete plugin->comp;
		plugin->comp = NULL;

		LOGDEBUG("triggering onloaded event");

		//todo fix
		plugin->FireEvent("onloaded", FB::variant_list_of(FB::variant(buffer)));

	} catch (std::exception &e)
	{
		LOGERROR("Caught Exception : %s", e.what());
		plugin->FireEvent("onerror", FB::variant_list_of(FB::variant(e.what())));
		plugin->status = plugin->comp->get_status();
		plugin->status.state = COMPUTER_FINISHED;
		delete plugin->comp;
		plugin->comp = NULL;
	} catch(...)
	{
		LOGERROR("Caught Exception Unknown");
		plugin->FireEvent("onerror", FB::variant_list_of(FB::variant("Undefined exception")));
		plugin->status = plugin->comp->get_status();
		plugin->status.state = COMPUTER_FINISHED;
		delete plugin->comp;
		plugin->comp = NULL;
	}

	LOGINFO("End async for dump");
	
	return(NULL);
}


void DiveBoardAPI::dump(const std::string& strport, const std::string& label)
{
	try {
		LOGINFO("Dump called with device %s on port %s", label.c_str(), strport.c_str());
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
			//CreateThread(NULL, 0, &DiveBoardAPI::dump_async, this, 0, NULL);
			boost::thread *th = new boost::thread( boost::bind(&DiveBoardAPI::dump_async, this));
			//pthread_t threads;
			//pthread_create(&threads, NULL, &DiveBoardAPI::dump_async, (void *) this);
		}
		else {
			dump_async((void*)this);
		}

	} catchall()
}


FB::variant DiveBoardAPI::detect(const std::string& computerType)
{
	try
	{
		ComputerFactory factory;
		std::string port = factory.detectConnectedDevice(computerType);
		return(FB::variant(port));

	} catch(DBException e)
	{
		LOGINFO("Caught Exception : %s", e.what());
	}
	return(FB::variant(""));
}

FB::VariantMap DiveBoardAPI::allports()
{
	FB::VariantMap ret;
	ComputerFactory factory;
	std::map<std::string, std::string> ports;

	ports = factory.allPorts();

	for (std::map<std::string, std::string>::iterator it = ports.begin(); it != ports.end(); ++it)
		ret[it->first] = FB::variant(it->second);

	return(ret);
}


FB::variant DiveBoardAPI::isComputerPluggedin()
{
	try
	{
		ComputerFactory factory;
		bool found = factory.isComputerPluggedin();
		return(FB::variant(found));

	} catch(DBException e)
	{
		LOGINFO("Caught Exception : %s", e.what());
	}
	return(FB::variant(false));
}


void DiveBoardAPI::cancel()
{
	if (comp) comp->cancel();
}

void DiveBoardAPI::setLogLevel(const std::string& level)
{
	Logger::setLogLevel(level);
}


void DiveBoardAPI::setLogSize(const unsigned long& size)
{
	Logger::logSize = size;
}
