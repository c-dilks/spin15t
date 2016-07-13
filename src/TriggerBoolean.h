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

const int n_tracks_max = 1000;

class TriggerBoolean : public TObject
{
  public:
    TriggerBoolean(Int_t stg1_in, Int_t stg2_in, Int_t mipn_in, Int_t use_tcu);
    const char * Name(Int_t idx0); // boolean index --> name
    Int_t Idx(char * name0); // boolean name --> index
    Bool_t Fired(Int_t idx0);
    Bool_t Fired(char * name0);
    Bool_t FiredAlternate(Int_t idx0, Int_t stg1_in, Int_t stg2_in, Int_t mipn_in, Int_t use_tcu);
    Bool_t FiredAlternate(char * name0, Int_t stg1_in, Int_t stg2_in, Int_t mipn_in, Int_t use_tcu);
    void PrintParameters() { printf("STG1=%d STG2=%d MIPN=%d RP_SOURCE=%d\n",STG1,STG2,MIPN,RP_SOURCE); };
    void Diagnostic(Int_t runnum0, Int_t event0);
    void CheckForTracks();

    // pointers to TCUbits and RPscint instances, set in the constructor
    TCUbits * TCU;
    RPscint * RPSCI;

    Int_t NBOOL;

    Float_t BBCvertex;


    // RP MuDST branches
    Int_t n_tracks,n_trackpoints;
    // track variables -- prefixed with t_
    Int_t t_index[n_tracks_max];
    Int_t t_branch[n_tracks_max];
    Int_t t_type[n_tracks_max];
    Int_t t_planesUsed[n_tracks_max];
    Double_t t_p[n_tracks_max];
    Double_t t_pt[n_tracks_max];
    Double_t t_eta[n_tracks_max];
    Double_t t_time[n_tracks_max];
    Double_t t_theta[n_tracks_max];
    Double_t t_thetaRP[n_tracks_max];
    Double_t t_phi[n_tracks_max];
    Double_t t_phiRP[n_tracks_max];
    Double_t t_t[n_tracks_max];
    Double_t t_xi[n_tracks_max];
    Bool_t t_gold[n_tracks_max];
    // trackpoint variables -- prefixed with p0_ and p1_
    Bool_t p_tpExists[2][n_tracks_max];
    Int_t p_RPid[2][n_tracks_max];
    Int_t p_quality[2][n_tracks_max];
    Double_t p_x[2][n_tracks_max];
    Double_t p_y[2][n_tracks_max];
    Double_t p_z[2][n_tracks_max];


  private:
    std::map<std::string, Int_t> trg_idx; // trigger boolean name to an index
    std::map<Int_t, std::string> trg_name; // trigger boolean index to name
    Bool_t EOR,WOR,IT,ET;

    Int_t n_good_tracks[4]; // [RP branch]

    Int_t STG1, STG2, MIPN, RP_SOURCE;

    Int_t STG1_tmp, STG2_tmp, MIPN_tmp, RP_SOURCE_tmp;

    ClassDef(TriggerBoolean,1);
};

#endif
