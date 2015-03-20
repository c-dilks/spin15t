#ifndef Trigger_
#define Trigger_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <map>

#include "TSystem.h"
#include "TObject.h"

class Trigger : public TObject
{
  public:
    Trigger();
    Int_t Mask(char * trigger0);
    Int_t Mask(Int_t num0);
    Int_t Index(char * trigger0);
    char * Name(Int_t num0);

    Int_t N;

  protected:
    std::map<Int_t, Int_t> mask_map;

    ClassDef(Trigger,1);
};

#endif
