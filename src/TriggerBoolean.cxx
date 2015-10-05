#include "TriggerBoolean.h"

ClassImp(TriggerBoolean)

namespace
{
  //------------------------------------------------------------------------------
  // trigger parameters: see RPscint.h for full documentation
  const Int_t STG1 = 0; // RPscint::track_trg strength
  const Int_t STG2 = 0; // RPscint::ud_track_trg strength
  const Int_t MIPN = 0; // RPscint::(trigger) mipn
  const Bool_t USE_TCU_BITS = 1; // if true, uses TCU bits EOR,WOR,IT,ET
                                 // instead of the strengthed booleans via QT
  //------------------------------------------------------------------------------

  enum ew_enum {kE,kW};
  enum io_enum {kI,kO};
  enum ud_enum {kU,kD};
  enum ns_enum {kN,kS};
  
  const TString sEW[2] = {"E","W"};
  const TString sIO[2] = {"I","O"};
  const TString sUD[2] = {"U","D"};
  const TString sNS[2] = {"N","S"};
};


TriggerBoolean::TriggerBoolean()
{
  TCU = new TCUbits();
  RPSCI = new RPscint();

  Int_t ii=0; 
  
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("N"),ii++));
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("EOR"),ii++));
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("WOR"),ii++));
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("SDE"),ii++));
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("SDW"),ii++));
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("ET"),ii++));
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("IT"),ii++));
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("DD"),ii++));
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("SDOR"),ii++));

  NBOOL = ii;

  std::map<std::string,Int_t>::iterator iter;
  for(iter=trg_idx.begin(); iter!=trg_idx.end(); ++iter)
  {
    trg_name.insert(std::pair<Int_t,std::string>(iter->second,iter->first));
  };
};


const char * TriggerBoolean::Name(Int_t idx0)
{
  std::string return_str;
  try { return_str = trg_name.at(idx0); }
  catch(const std::out_of_range& e)
  {
    fprintf(stderr,"ERROR: trg_name out of range\n");
    return "";
  };
  return return_str.data();
};


Int_t TriggerBoolean::Idx(char * name0)
{
  Int_t return_idx;
  try { return_idx = trg_idx.at(std::string(name0)); }
  catch(const std::out_of_range& e)
  {
    fprintf(stderr,"ERROR: trg_idx out of range\n");
    return -1;
  };
  return return_idx;
};


Bool_t TriggerBoolean::Fired(Int_t idx0)
{
  std::string name0;
  try { name0 = trg_name.at(idx0); }
  catch(const std::out_of_range& e)
  {
    fprintf(stderr,"ERROR: RP idx out of range\n");
    return 0;
  };
  return Fired((char*)(name0.data()));
};



Bool_t TriggerBoolean::Fired(char * name0)
{
  if(!strcmp(name0,"N")) return true; // no RP bias
  else
  {
    if(USE_TCU_BITS)
    {
      EOR = TCU->Fired("RP_EOR");
      WOR = TCU->Fired("RP_WOR");
      ET = TCU->Fired("RP_ET");
      IT = TCU->Fired("RP_IT");
    }
    else
    {
      RPSCI->Process();
      EOR = RPSCI->track_trg[kE][STG1][MIPN];
      WOR = RPSCI->track_trg[kW][STG1][MIPN];
      ET = RPSCI->elastic_trg[STG2][MIPN];
      IT = RPSCI->inelastic_trg[STG2][MIPN];
    };

    if(!strcmp(name0,"EOR")) return EOR;
    else if(!strcmp(name0,"WOR")) return WOR;
    else if(!strcmp(name0,"ET")) return ET;
    else if(!strcmp(name0,"IT")) return IT;
    else if(!strcmp(name0,"SDOR")) return EOR || WOR;
    else
    {
      if(!strcmp(name0,"SDE")) 
        return EOR &&
               !(TCU->Fired("ZDC-E")) && !(TCU->Fired("BBC-E")) &&
              (  TCU->Fired("ZDC-W")  ||   TCU->Fired("BBC-W")  );
      else if(!strcmp(name0,"SDW")) 
        return WOR &&
               !(TCU->Fired("ZDC-W")) && !(TCU->Fired("BBC-W")) &&
              (  TCU->Fired("ZDC-E")  ||   TCU->Fired("BBC-E")  );
      else if(!strcmp(name0,"DD"))
        return EOR && WOR && TCU->FiredTOF() && !(TCU->FiredBBC());
    };
  };
  fprintf(stderr,"ERROR: unrecognised trigger boolean\n");
  return false;
};

