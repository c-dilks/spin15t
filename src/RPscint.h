// reads PP001 QT data, which are roman pot scintillator 
// ADCs and TACs

#ifndef RPscint_
#define RPscint_
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "TSystem.h"
#include "TObject.h"
#include "TString.h"

class RPscint : public TObject
{
  public:
    RPscint();
    Bool_t Process(); // return true if NQT>0

    Int_t iEW(Int_t idx0) { return (Int_t)((idx0>>3)&0x1); };
    Int_t iIO(Int_t idx0) { return (Int_t)((idx0>>2)&0x1); };
    Int_t iUD(Int_t idx0) { return (Int_t)((idx0>>1)&0x1); };
    Int_t iNS(Int_t idx0) { return (Int_t)((idx0>>0)&0x1); };
    void IdxToEiun(Int_t idx0, Int_t &ew0, Int_t &io0, Int_t &ud0, Int_t &ns0);
    Int_t EiunToIdx(Int_t ew0, Int_t io0, Int_t ud0, Int_t ns0);
    TString RPname(Int_t idx0);

    //---------------------------------------------------------
    // EVENT VARIABLES
    Short_t N[2]; // [0=east 1=west]
    Short_t Idx[2][16]; // [east/west] [sorted # (max 16)]
    Short_t ADC[2][16];
    Short_t TAC[2][16];
    Float_t vertex;
    //---------------------------------------------------------
    
    
    Double_t MIPthresh[16];

    // trigger bits
    Bool_t track_trg[2][4][3]; // [e/w] [strength] [mipn]
     /* strength:
      * 0 = completely unrestricted: either inner or outer fires 
      *     (mip not selected for strenth 0)
      * 1 = i/o restricted: any inner fires any outer
      * 2 = u/d restricted: inner u(d) fires outer u(d)
      * 3 = n/s restricted: inner u(d) fires outer u(d)
      *                           inner n(s) fires outer n(s)
      *
      * mipn:
      * 0 = unrestricted
      * 1 = inner channel in mip region
      * 2 = inner channel adc > mip region
      */

    Bool_t ud_track_trg[2][2][2][3]; // [e/w] [u/d] [strength] [mipn]
     /* strength:
      * 0 = inner u(d) fires outer u(d)
      * 1 = n/s restricted: also inner n(s) fires outer n(s)
      */

    Bool_t elastic_trg[2][3]; // [strength adopted from ud_track_trg] [mipn]
    Bool_t inelastic_trg[2][3]; // [strength adopted from ud_track_trg] [mipn]


  private:
    Int_t ch,ew,io,ud,ns,q,stg;
    Int_t udi,udo,nsi,nso;
    Bool_t fired[16];
    Double_t ADCtmp[16];
    Int_t ii,oo,mipn;

    ClassDef(RPscint,1);
};

#endif
