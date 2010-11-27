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

    // Read/Write property ${PROPERTY.ident}
    std::string get_testString();
    void set_testString(const std::string& val);

    // Read-only property ${PROPERTY.ident}
    std::string get_version();

	std::string get_logs();

	// Method echo
    FB::variant echo(const FB::variant& msg);
    
    // Method test-event
    void testEvent(const FB::variant& s);

	void extract(const std::string& labal, const std::string& sport);
	void detect();
	int get_nbDivesRead();
	int get_nbDivesTotal();
	std::string get_status();


private:
    DiveBoardWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;
    std::string m_testString;

	Computer *comp;
	ComputerStatus status;
	bool alwaysAsync;
	static HANDLE mutex;

	//remove for MAC
	//static DWORD WINAPI asyncfunc(LPVOID lpParam);
	static void *asyncfunc(void*);
};

#endif // H_DiveBoardAPI

