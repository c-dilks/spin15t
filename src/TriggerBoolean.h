// main class for defining trigger booleans using TCUbits and RPscint members

#ifndef TriggerBoolean_
#define TriggerBoolean_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <map>
#include <stdexcept>

#include "TSystem.h"
#include "TObject.h"
#include "TTree.h"
#include "TString.h"
#include "Environ.h"

#include "TCUbits.h"
#include "RPscint.h"

class TriggerBoolean : public TObject
{
  public:
    TriggerBoolean();
    const char * Name(Int_t idx0); // boolean index --> name
    Int_t Idx(char * name0); // boolean name --> index
    Bool_t Fired(Int_t idx0);
    Bool_t Fired(char * name0);

    // pointers to TCUbits and RPscint instances, set in the constructor
    TCUbits * TCU;
    RPscint * RPSCI;

    Int_t NBOOL;


  private:
    std::map<std::string, Int_t> trg_idx; // trigger boolean name to an index
    std::map<Int_t, std::string> trg_name; // trigger boolean index to name
    Bool_t EOR,WOR,IT,ET;

    ClassDef(TriggerBoolean,1);
};

#endif
