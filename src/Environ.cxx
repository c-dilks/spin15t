#include "Environ.h"
using namespace std;

ClassImp(Environ)

Environ::Environ()
{
  if(gSystem->Getenv("SPINDIR")==NULL)
  {
    fprintf(stderr,"ERROR: source env variables\n");
    return;
  };

  sscanf(gSystem->Getenv("SPINDIR"),"%s",SpinDir);
  sscanf(gSystem->Getenv("TRIGGER_TYPE"),"%s",TriggerType);
  sscanf(gSystem->Getenv("MASS_CUT_TYPE"),"%s",MassCutType);
  sscanf(gSystem->Getenv("RP_SELECT"),"%s",RPselect);

  sscanf(gSystem->Getenv("PHI_BINS"),"%d",&PhiBins);
  sscanf(gSystem->Getenv("ETA_BINS"),"%d",&EtaBins);
  sscanf(gSystem->Getenv("EN_BINS"),"%d",&EnBins);
  sscanf(gSystem->Getenv("PT_BINS"),"%d",&PtBins);

  sscanf(gSystem->Getenv("PHI_LOW"),"%f",&PhiLow);
  sscanf(gSystem->Getenv("ETA_LOW"),"%f",&EtaLow);
  sscanf(gSystem->Getenv("EN_LOW"),"%f",&EnLow);
  sscanf(gSystem->Getenv("PT_LOW"),"%f",&PtLow);
  
  sscanf(gSystem->Getenv("PHI_HIGH"),"%f",&PhiHigh);
  sscanf(gSystem->Getenv("ETA_HIGH"),"%f",&EtaHigh);
  sscanf(gSystem->Getenv("EN_HIGH"),"%f",&EnHigh);
  sscanf(gSystem->Getenv("PT_HIGH"),"%f",&PtHigh);

  sscanf(gSystem->Getenv("STG1"),"%d",&STG1);
  sscanf(gSystem->Getenv("STG2"),"%d",&STG2);
  sscanf(gSystem->Getenv("MIPN"),"%d",&MIPN);
  sscanf(gSystem->Getenv("RP_SOURCE"),"%d",&RP_SOURCE);
  
  Float_t d;
  char s[32];
  for(Int_t b=0; b<=PhiBins; b++) 
  {
    sprintf(s,"PHI_DIV_%d",b);
    sscanf(gSystem->Getenv(s),"%f",&d);
    PhiDivMap.insert(std::pair<Int_t,Float_t>(b,d));
  };
  for(Int_t b=0; b<=EtaBins; b++) 
  {
    sprintf(s,"ETA_DIV_%d",b);
    sscanf(gSystem->Getenv(s),"%f",&d);
    EtaDivMap.insert(std::pair<Int_t,Float_t>(b,d));
  };
  for(Int_t b=0; b<=EnBins; b++) 
  {
    sprintf(s,"EN_DIV_%d",b);
    sscanf(gSystem->Getenv(s),"%f",&d);
    EnDivMap.insert(std::pair<Int_t,Float_t>(b,d));
  };
  for(Int_t b=0; b<=PtBins; b++) 
  {
    sprintf(s,"PT_DIV_%d",b);
    sscanf(gSystem->Getenv(s),"%f",&d);
    PtDivMap.insert(std::pair<Int_t,Float_t>(b,d));
  };
};

Float_t Environ::PhiDiv(Int_t bin) 
  { if(bin>=0 && bin<=PhiBins) return PhiDivMap.at(bin); return Error("Phi");};
Float_t Environ::EtaDiv(Int_t bin) 
  { if(bin>=0 && bin<=EtaBins) return EtaDivMap.at(bin); return Error("Eta");};
Float_t Environ::EnDiv(Int_t bin) 
  { if(bin>=0 && bin<=EnBins) return EnDivMap.at(bin); return Error("En");};
Float_t Environ::PtDiv(Int_t bin) 
  { if(bin>=0 && bin<=PtBins) return PtDivMap.at(bin); return Error("Pt");};

Double_t Environ::Error(const char * bintype)
{
  fprintf(stderr,"ERROR: invalid %s bin number\n",bintype);
  return 0;
};
