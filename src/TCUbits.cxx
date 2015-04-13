// Implementation of L1 trigger bits (from TCU input)
// -- see ../TCUbits directory for further information

#include "TCUbits.h"

ClassImp(TCUbits)

using namespace std;

TCUbits::TCUbits()
{
  // open tcu bit tables
  env = new Environ();
  char tcu_tr_file[512];
  char tcuchan_tr_file[512];
  sprintf(tcu_tr_file,"%s/TCUbits/tcu.dat",env->SpinDir);
  sprintf(tcuchan_tr_file,"%s/TCUbits/tcuchan.dat",env->SpinDir);
  tcu_tr = new TTree();
  tcuchan_tr = new TTree();
  tcu_tr->ReadFile(tcu_tr_file,"dsm/C:bit/i:trigger/C");
  tcuchan_tr->ReadFile(tcuchan_tr_file,"dsm/C:tcuchan/i");
  UInt_t bit,tcuchan;
  char dsm[16];
  char trig[32];
  tcu_tr->SetBranchAddress("dsm",dsm);
  tcu_tr->SetBranchAddress("bit",&bit);
  tcu_tr->SetBranchAddress("trigger",trig);
  tcuchan_tr->SetBranchAddress("dsm",dsm);
  tcuchan_tr->SetBranchAddress("tcuchan",&tcuchan);


  // fill tcu_bit map :: trigger --> (DSM,bit)
  for(Int_t i=0; i<tcu_tr->GetEntries(); i++)
  {
    tcu_tr->GetEntry(i);
    tcu_bit.insert(pair<string,pair<string,UInt_t> >(string(trig),pair<string,UInt_t>(string(dsm),bit)));
  };

  // fill tcu_chan map :: DSM --> TCU channel
  for(Int_t i=0; i<tcuchan_tr->GetEntries(); i++)
  {
    tcuchan_tr->GetEntry(i);
    tcu_chan.insert(pair<string,UInt_t>(string(dsm),tcuchan));
  };
  debug=false;
};


void TCUbits::SetBits(UInt_t inbits[8])
{
  for(Int_t i=0; i<8; i++) thisdsm[i] = inbits[i];
};


Bool_t TCUbits::Fired(char * trg)
{
  UInt_t tcuchan0,bit0;
  string dsm0;
  try
  {
    dsm0 = tcu_bit.at(string(trg)).first;
    bit0 = tcu_bit.at(string(trg)).second;
    tcuchan0 = tcu_chan.at(dsm0);
  }
  catch(const out_of_range& e)
  {
    fprintf(stderr,"ERROR: invalid trigger requested in TCUbits::Fired\n");
    return 0;
  };
  if(debug) printf("trigger=%s DSM=%s bit=%d TCUchan=%d\n",trg,dsm0.data(),bit0,tcuchan0);
  return (thisdsm[tcuchan0] >> bit0) & 1;
};


// RP triggers
//////////////////////
/*

EOR = E1U | E1D | E2U | E2D 
WOR = W1U | W1D | W2U | W2D 
 
Elastic Trigger:    ET = [ (E1U | E2U) & (W1D | W2D) ] | [ (E1D | E2D) & (W1U | W2U) ]
Inelastic Trigger:  IT = [ (E1U | E2U) & (W1U | W2U) ] | [ (E1D | E2D) & (W1D | W2D) ] 

Single Diffraction to E: SDE = WOR & !ZDCW & !BBCW & (ZDCE | BBCE)
Single Diffraction to W: SDW = EOR & !ZDCE & !BBCE & (ZDCW | BBCW) 

Double Diffractive: DD = EOR & WOR & TOF & !BBC & !FMS 
                         (currently not including !FMS term)

[ In TCU inputs: EOR,WOR,ET,IT ]

*/
Bool_t TCUbits::RP_EOR() { return Fired("RP_EOR"); };
Bool_t TCUbits::RP_WOR() { return Fired("RP_WOR"); };
Bool_t TCUbits::RP_ET() { return Fired("RP_ET"); };
Bool_t TCUbits::RP_IT() { return Fired("RP_IT"); };
Bool_t TCUbits::RP_SDE()
{
  return (Fired("RP_WOR") &&
         !(Fired("ZDC-W")) && !(Fired("BBC-W")) &&
          (Fired("ZDC-E") || Fired("BBC-E")));
};
Bool_t TCUbits::RP_SDW()
{
  return (Fired("RP_EOR") &&
         !(Fired("ZDC-E")) && !(Fired("BBC-E")) &&
          (Fired("ZDC-W") || Fired("BBC-W")));
};
//Bool_t TCUbits::RP_DD()


// TOF trigger; returns true if anything seen in TOF
Bool_t TCUbits::FiredTOF()
{
  return (Fired("TOF_UPC") ||
          Fired("TOFmult0") ||
          Fired("TOFmult1") ||
          Fired("TOFmult2") ||
          Fired("TOFmult3") ||
          Fired("TOFsector0_3") ||
          Fired("TOFsector1_4") ||
          Fired("TOFsector2_5"));
};

// BBC trigger; returns true if anything seen in BBC
Bool_t TCUbits::FiredBBC()
{
  return (Fired("BBC-E") || Fired("BBC-W"));
};
