#include "TriggerBoolean.h"

ClassImp(TriggerBoolean)

namespace
{
  enum ew_enum {kE,kW};
  enum io_enum {kI,kO};
  enum ud_enum {kU,kD};
  enum ns_enum {kN,kS};

  enum rp_source_enum { 
    kMuDSTs=0,
    kTCUbits=1,
    kQTs=2
  };
  enum rp_branch_enum { 
    kEU=0,
    kED=1,
    kWU=2,
    kWD=3
  };
  
  const TString sEW[2] = {"E","W"};
  const TString sIO[2] = {"I","O"};
  const TString sUD[2] = {"U","D"};
  const TString sNS[2] = {"N","S"};
};


TriggerBoolean::TriggerBoolean(Int_t stg1_in, Int_t stg2_in, Int_t mipn_in, Int_t rp_source_in)
{
  STG1 = stg1_in;
  STG2 = stg2_in;
  MIPN = mipn_in;
  RP_SOURCE = rp_source_in;
   
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
  //trg_idx.insert(std::pair<std::string,Int_t>(std::string("DD"),ii++));
  trg_idx.insert(std::pair<std::string,Int_t>(std::string("SDOR"),ii++));

  NBOOL = ii;

  std::map<std::string,Int_t>::iterator iter;
  for(iter=trg_idx.begin(); iter!=trg_idx.end(); ++iter)
  {
    trg_name.insert(std::pair<Int_t,std::string>(iter->second,iter->first));
  };

  BBCvertex=0;
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
    if(RP_SOURCE==kMuDSTs) 
    {
      CheckForTracks();
    }
    else if(RP_SOURCE==kTCUbits)
    {
      EOR = TCU->Fired("RP_EOR");
      WOR = TCU->Fired("RP_WOR");
      ET = TCU->Fired("RP_ET");
      IT = TCU->Fired("RP_IT");
    }
    else if(RP_SOURCE==kQTs)
    {
      RPSCI->Process();
      EOR = RPSCI->track_trg[kE][STG1][MIPN];
      WOR = RPSCI->track_trg[kW][STG1][MIPN];
      ET = RPSCI->elastic_trg[STG2][MIPN];
      IT = RPSCI->inelastic_trg[STG2][MIPN];
    }
    else
    {
      fprintf(stderr,"ERROR: unknown RP_SOURCE\n");
      return false;
    };

    if(!strcmp(name0,"EOR")) return EOR;
    else if(!strcmp(name0,"WOR")) return WOR;
    else if(!strcmp(name0,"ET")) return ET;
    else if(!strcmp(name0,"IT")) return IT;
    else if(!strcmp(name0,"SDOR")) return (EOR || WOR);
    else
    {
      if(!strcmp(name0,"SDE")) 
        return (EOR &&
                !(TCU->Fired("ZDC-E")) && !(TCU->Fired("BBC-E")) &&
               (  TCU->Fired("ZDC-W")  ||   TCU->Fired("BBC-W")  ));
      else if(!strcmp(name0,"SDW")) 
        return (WOR &&
                !(TCU->Fired("ZDC-W")) && !(TCU->Fired("BBC-W")) &&
               (  TCU->Fired("ZDC-E")  ||   TCU->Fired("BBC-E")  ));
      else if(!strcmp(name0,"DD"))
        return (EOR && WOR && TCU->FiredTOF() && !(TCU->FiredBBC()));
    };
  };
  fprintf(stderr,"ERROR: unrecognised trigger boolean\n");
  return false;
};


Bool_t TriggerBoolean::FiredAlternate(Int_t idx0, 
                                      Int_t stg1_in, Int_t stg2_in, Int_t mipn_in, Int_t rp_source_in)
{
  std::string name0;
  try { name0 = trg_name.at(idx0); }
  catch(const std::out_of_range& e)
  {
    fprintf(stderr,"ERROR: RP idx out of range\n");
    return 0;
  };
  return FiredAlternate((char*)(name0.data()),stg1_in,stg2_in,mipn_in,rp_source_in);
};


// changes trigger strength / mip parameters to alternate values, checks if that fired, 
// then changes the parameters back (note that the parameters are initialised in the constructor,
// and since they're private, cannot be changed outside the class)
Bool_t TriggerBoolean::FiredAlternate(char * name0, 
                                      Int_t stg1_in, Int_t stg2_in, Int_t mipn_in, Int_t rp_source_in)
{
  STG1_tmp = STG1;
  STG2_tmp = STG2;
  MIPN_tmp = MIPN;
  RP_SOURCE_tmp = RP_SOURCE;
  
  STG1 = stg1_in;
  STG2 = stg2_in;
  MIPN = mipn_in;
  RP_SOURCE = rp_source_in;

  Bool_t return_val = Fired(name0);

  STG1 = STG1_tmp;
  STG2 = STG2_tmp;
  MIPN = MIPN_tmp;
  RP_SOURCE = RP_SOURCE_tmp;

  return return_val;
};


void TriggerBoolean::Diagnostic(Int_t runnum0, Int_t event0)
{
  /*
   PrintParameters();
   printf("NE=%d NW=%d\n",RPSCI->N[kE],RPSCI->N[kW]);
   */

  for(int eee=0; eee<2; eee++)
  {
    for(int zz=0; zz<8; zz++)
    {
      if(RPSCI->ADC[eee][zz]>0)
      {
        // runnum, eventnum, NE NW ew Idx ADC TAC TCU_EOR TCU_WOR TCU_ET TCU_IT TCUlike_EOR ... mismatch
        printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f\n",
            runnum0,event0,
            RPSCI->N[kE],RPSCI->N[kW],eee,
            RPSCI->Idx[eee][zz],RPSCI->ADC[eee][zz],RPSCI->TAC[eee][zz],
            TCU->Fired("RP_EOR"),TCU->Fired("RP_WOR"),TCU->Fired("RP_ET"),TCU->Fired("RP_IT"),
            EOR,WOR,ET,IT,
            (EOR==TCU->Fired("RP_EOR"))&&(WOR==TCU->Fired("RP_WOR"))&&(ET==TCU->Fired("RP_ET"))&&(IT==TCU->Fired("RP_IT")),
            BBCvertex);
        /*
           printf("ee=%d %d %d %d\n",
           eee,(Int_t)(RPSCI->Idx[eee][zz]),
           (Int_t)(RPSCI->ADC[eee][zz]),
           (Int_t)(RPSCI->TAC[eee][zz]));
           */
      };
    };
  };

  /*
     printf("case: EOR=%d WOR=%d ET=%d IT=%d\n",EOR,WOR,ET,IT);
     printf("TCU:  EOR=%d WOR=%d ET=%d IT=%d\n",
     TCU->Fired("RP_EOR"),TCU->Fired("RP_WOR"),TCU->Fired("RP_ET"),TCU->Fired("RP_IT"));
     printf("allmatch=%d EOR-%d WOR-%d ET-%d IT-%d\n",
     (EOR==TCU->Fired("RP_EOR"))&&(WOR==TCU->Fired("RP_WOR"))&&(ET==TCU->Fired("RP_ET"))&&(IT==TCU->Fired("RP_IT")),
     EOR==TCU->Fired("RP_EOR"),WOR==TCU->Fired("RP_WOR"),ET==TCU->Fired("RP_ET"),IT==TCU->Fired("RP_IT"));
     printf("\n");
     */
  //-----------
};


// uses MuDST branches to look for valid tracks, and assignes values to EOR,WOR,ET, and IT
void TriggerBoolean::CheckForTracks() {
  // reset booleans
  EOR = false;
  WOR = false;
  IT = false;
  ET = false;

  // reset n_good_tracks[RP branch]
  for(int b=0; b<4; b++) n_good_tracks[b] = 0;

  if(n_tracks>0) {
    for(int x=0; x<n_tracks; x++) {
      // GOOD TRACKS CUT---------------------------------
      if(t_gold[x] && t_p[x]>40.0 && t_p[x]<100) {
        if(t_branch[x]>=0 && t_branch[x]<=4) {
          n_good_tracks[t_branch[x]]++;
        }
        else {
          fprintf(stderr,"ERROR: t_branch not in range!!!\n");
          return;
        };
      };
      //-------------------------------------------------
    };

    // BOOLEANS CUTS
    // single good track only -- highly restrictive
    /*
    if((n_good_tracks[kEU]==1 && n_good_tracks[kED]==0) ||
       (n_good_tracks[kEU]==0 && n_good_tracks[kED]==1))
       EOR = true;
    if((n_good_tracks[kWU]==1 && n_good_tracks[kWD]==0) ||
       (n_good_tracks[kWU]==0 && n_good_tracks[kWD]==1)) 
       WOR = true;
    if((n_good_gracks[kEU]==1 && n_good_tracks[kED]==0 && n_good_tracks[kWU]==0 && n_good_tracks[kWD]==1) ||
       (n_good_gracks[kEU]==0 && n_good_tracks[kED]==1 && n_good_tracks[kWU]==1 && n_good_tracks[kWD]==0))
       ET = true;
    if((n_good_gracks[kEU]==1 && n_good_tracks[kED]==0 && n_good_tracks[kWU]==1 && n_good_tracks[kWD]==0) ||
       (n_good_gracks[kEU]==0 && n_good_tracks[kED]==1 && n_good_tracks[kWU]==0 && n_good_tracks[kWD]==1))
       IT = true;
    */
    // nonzero good tracks -- highly unrestrictive
    ///*
    if(n_good_tracks[kEU]>0 || n_good_tracks[kED]>0) EOR = true;
    if(n_good_tracks[kWU]>0 || n_good_tracks[kWD]>0) WOR = true;
    if((n_good_tracks[kEU]>0 && n_good_tracks[kWD]>0) || 
       (n_good_tracks[kED]>0 && n_good_tracks[kWU]>0)) ET = true;
    if((n_good_tracks[kEU]>0 && n_good_tracks[kWU]>0) || 
       (n_good_tracks[kED]>0 && n_good_tracks[kWD]>0)) IT = true;
    //*/
  };
};
