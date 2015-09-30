#include "RPscint.h"

ClassImp(RPscint)

namespace
{
  enum ew_enum {kE,kW};
  enum io_enum {kI,kO};
  enum ud_enum {kU,kD};
  enum ns_enum {kN,kS};
  
  const TString sEW[2] = {"E","W"};
  const TString sIO[2] = {"I","O"};
  const TString sUD[2] = {"U","D"};
  const TString sNS[2] = {"N","S"};
};


RPscint::RPscint()
{
  for(ch=0; ch<16; ch++) printf("%s %d\n",RPname(ch).Data(),ch);

  // set default values
  for(ew=0; ew<2; ew++)
  {
    N[ew]=0;
    for(ch=0; ch<16; ch++)
    {
      Idx[ew][ch]=0;
      ADC[ew][ch]=0;
      TAC[ew][ch]=0;
    };
  };
  vertex=0;

  // MIP region ADC thresholds
  MIPthresh[0]  = 230;
  MIPthresh[1]  = 230;
  MIPthresh[2]  = 230;
  MIPthresh[3]  = 240;
  MIPthresh[4]  = 230;
  MIPthresh[5]  = 160;
  MIPthresh[6]  = 250;
  MIPthresh[7]  = 250;
  MIPthresh[8]  = 250;
  MIPthresh[9]  = 230;
  MIPthresh[10] = 220;
  MIPthresh[11] = 240;
  MIPthresh[12] = 300;
  MIPthresh[13] = 125;
  MIPthresh[14] = 240;
  MIPthresh[15] = 260;
};

Bool_t RPscint::Process()
{
  if(N[kE]==0 && N[kW]==0) return false;
  
  // reset bits
  memset(fired,0,sizeof(fired));
  for(ew=0; ew<2; ew++)
  {
    for(stg=0; stg<4; stg++)
    {
      for(mipn=0; mipn<3; mipn++)
      {
        track_trg[ew][stg][mipn]=0;
        if(stg<2)
        {
          for(ud=0; ud<2; ud++)
          {
            ud_track_trg[ew][ud][stg][mipn]=0;
          };
        };
      };
    };
  };


  // loop through hit channels
  for(ew=0; ew<2; ew++)
  {
    for(q=0; q<N[ew]; q++)
    {
      fired[Idx[ew][q]] = true;
      ADCtmp[Idx[ew][q]] = ADC[ew][q];
    };
  };


  // track trigger bits
  for(ew=0; ew<2; ew++)
  {
    if(N[ew]>0) 
    {
      /* stg 0 track_trig same for all mipn */
      for(mipn=0; mipn<3; mipn++)
        track_trg[ew][0][mipn]=1;
    };

    // loop through inner seqs
    for(udi=0; udi<2; udi++)
    {
      for(nsi=0; nsi<2; nsi++)
      {
        // loop through outer seqs if inner seq fired
        ii = EiunToIdx(ew,kI,udi,nsi);
        if(fired[ii])
        {
          mipn = (ADCtmp[ii] < MIPthresh[ii]) ? 1:2;
          for(udo=0; udo<2; udo++)
          {
            for(nso=0; nso<2; nso++)
            {
              // if outer seq fired too, check it's alignment
              // w.r.t. inner seq
              oo = EiunToIdx(ew,kO,udo,nso);
              if(fired[oo]) 
              {
                track_trg[ew][1][0]=1;
                track_trg[ew][1][mipn]=1;

                // u/d I hits u/d O
                if(udo==udi)
                {
                  track_trg[ew][2][0]=1;
                  track_trg[ew][2][mipn]=1;
                  ud_track_trg[ew][udo][0][0]=1;
                  ud_track_trg[ew][udo][0][mipn]=1;

                  // n/s I hits n/s O
                  if(nso==nsi)
                  {
                    track_trg[ew][3][0]=1;
                    track_trg[ew][3][mipn]=1;
                    ud_track_trg[ew][udo][1][0]=1;
                    ud_track_trg[ew][udo][1][mipn]=1;
                  };
                };
              }; // eo if outer seq fired
            };
          };
        }; // eo if innner seq fired
      };
    }; // eo inner seq loop
  }; // eo e/w loop


  // elatic and inelastic triggers
  for(stg=0; stg<2; stg++)
  {
    for(mipn=0; mipn<3; mipn++)
    {
      elastic_trg[stg][mipn] = (ud_track_trg[kE][kU][stg][mipn] && ud_track_trg[kW][kD][stg][mipn]) ||
                               (ud_track_trg[kE][kD][stg][mipn] && ud_track_trg[kW][kU][stg][mipn]);
      inelastic_trg[stg][mipn] = (ud_track_trg[kE][kU][stg][mipn] && ud_track_trg[kW][kU][stg][mipn]) ||
                                 (ud_track_trg[kE][kD][stg][mipn] && ud_track_trg[kW][kD][stg][mipn]);
    };
  };

  return true;
};

void RPscint::IdxToEiun(Int_t idx0, Int_t &ew0, Int_t &io0, Int_t &ud0, Int_t &ns0)
{
  ew0 = iEW(idx0);
  io0 = iIO(idx0);
  ud0 = iUD(idx0);
  ns0 = iNS(idx0);
};
  
Int_t RPscint::EiunToIdx(Int_t ew0, Int_t io0, Int_t ud0, Int_t ns0)
{
  return 8*ew0 + 4*io0 + 2*ud0 + ns0;
};

TString RPscint::RPname(Int_t idx0)
{
  return sEW[iEW(idx0)] + sIO[iIO(idx0)] + sUD[iUD(idx0)] + sNS[iNS(idx0)];
};
