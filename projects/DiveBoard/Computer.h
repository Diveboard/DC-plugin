#pragma once

#include "stdafx.h"
#include <string>

typedef enum {
  COMPUTER_MODEL_UNKNOWN=0,
  SUUNTO_MODEL_OLD_SPYDER,
  SUUNTO_MODEL_NEW_SPYDER,
  SUUNTO_MODEL_OLD_COBRA_OR_VYPER,
  SUUNTO_MODEL_NEW_VYPER,
  SUUNTO_MODEL_VYTEC,
  SUUNTO_MODEL_STINGER,
  SUUNTO_MODEL_MOSQUITO,
  MARES_MODEL_M2
} ComputerModel;

class DiveData{
public:
  double O2;
  double start_pressure;
  double end_pressure;
  double altitude;
  double max_temperature;
  double min_temperature;
  double air_temperature;
  double max_depth;
  int repnum;
  int surface_hour;
  int surface_min;
  long duration;
	std::string date;
	std::string time;
	std::string profile;
} ;

typedef enum {
	COMPUTER_NOT_STARTED,
	COMPUTER_RUNNING,
	COMPUTER_FINISHED
} ComputerState;

typedef struct {
	int nbDivesRead;
	int nbDivesTotal;
	ComputerState state;
	int percent;
} ComputerStatus;



class Computer
{
	HANDLE mutex;
protected:
	virtual int _get_all_dives(std::string &xml) {return(-1);};
	virtual ComputerModel _get_model() {return(COMPUTER_MODEL_UNKNOWN);};
public:
	Computer(){};
	virtual ~Computer(void){};
	//virtual int test()=0;
	int get_all_dives(std::string &xml) {return(_get_all_dives(xml));};
	ComputerModel get_model() { return(_get_model());};
	virtual ComputerStatus get_status() {ComputerStatus t; t.nbDivesRead=0; t.nbDivesTotal=0; t.state = COMPUTER_NOT_STARTED; return(t);};
	virtual void cancel() { }
};

