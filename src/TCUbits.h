// Implementation of L1 trigger bits (from TCU input)
// -- see ../TCUbits directory for further information

#ifndef TCUbits_
#define TCUbits_

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

class TCUbits : public TObject
{
  public:
    TCUbits();
    void SetBits(UInt_t inbits[8]);
    Bool_t Fired(char * trg);

    Bool_t RP_EOR();
    Bool_t RP_WOR();
    Bool_t RP_ET();
    Bool_t RP_IT();
    Bool_t RP_SDE();
    Bool_t RP_SDW();
    //Bool_t RP_DD();

    Bool_t FiredTOF();
    Bool_t FiredBBC();


    Bool_t debug;

  private:
    Environ * env;
    TTree * tcu_tr;
    TTree * tcuchan_tr;
    UInt_t thisdsm[8];

    std::map<std::string, std::pair<std::string,UInt_t> > tcu_bit; // trigger --> (DSM,input bit)
    std::map<std::string, UInt_t> tcu_chan; // DSM --> TCU channel

    ClassDef(TCUbits,1);
};

#endif
