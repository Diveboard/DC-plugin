/**********************************************************\

  Auto-generated DiveBoardAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "DiveBoard.h"
#include "Computer.h"
#include "Device.h"


#ifndef H_DiveBoardAPI
#define H_DiveBoardAPI

class DiveBoardAPI : public FB::JSAPIAuto
{
public:
    DiveBoardAPI(DiveBoardPtr plugin, FB::BrowserHostPtr host);
    virtual ~DiveBoardAPI();

    DiveBoardPtr getPlugin();

    // Read-only property ${PROPERTY.ident}
    std::string get_version();
	std::string get_name();
	std::string get_nodename();
	FB::VariantMap get_status();
	std::string get_logs();

	// Method echo
    FB::variant echo(const FB::variant& msg);
	void extract(const std::string& labal, const std::string& sport);
	FB::variant detect(const std::string& computerType);
	FB::VariantMap allports();
	FB::variant isComputerPluggedin();
	void cancel();
	void setLogLevel(const std::string&);
	void setLogSize(const unsigned long& size);
private:
    DiveBoardWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;
    std::string m_testString;

	Computer *comp;
	ComputerStatus status;
	bool alwaysAsync;
	static HANDLE mutex;

	static void *asyncfunc(void*);
};

#endif // H_DiveBoardAPI

