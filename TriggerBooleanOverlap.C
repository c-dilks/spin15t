// checks overlap between various trigger booleans with each other and with the FMS

void TriggerBooleanOverlap(const char * set_name = "080ac")
{
  const Int_t MAX_RUNS = 12; // assumed max number of runs in a set file

  // class instantiation
  gSystem->Load("src/RunInfo.so");
  RunInfo * RD = new RunInfo();
  LevelTwo * T = new LevelTwo();
  Environ * env = new Environ();
  EventClass * ev = new EventClass();
  TriggerBoolean * trg_bool = 
    new TriggerBoolean(env->STG1,env->STG2,env->MIPN,env->USE_TCU_BITS);

  Int_t evc = ev->Idx("pi0"); // choose an event class
  //char l2trg[32]; strcpy(l2trg,"All"); // choose FMS L2 trigger mask (FMS OR = "All")
  char l2trg[32]; strcpy(l2trg,"JP2");

  int i,j;
  enum ew_enum {kE,kW};
  enum io_enum {kI,kO};
  enum ud_enum {kU,kD};
  enum ns_enum {kN,kS};
  enum lr_enum {kL,kR};


  // open tree from redset file
  char infile_full_name[256];
  sprintf(infile_full_name,"redset/RedOutputset%s.root",set_name);
  TFile * infile = new TFile(infile_full_name,"READ");
  TTree * tr = (TTree*) infile->Get("str");

  Float_t E12,Pt,Eta,Phi,M12,Z,b_pol,y_pol;
  Bool_t kicked,isConsistent;
  Int_t runnum,bx;
  UInt_t L2sum[2];
  UInt_t lastdsm[8];
  Float_t N12;

  Float_t E12_min,Pt_min,Eta_min,Phi_min;
  Float_t E12_max,Pt_max,Eta_max,Phi_max;
  E12_min=Pt_min=Eta_min=Phi_min=1000;
  E12_max=Pt_max=Eta_max=Phi_max=0;

  tr->SetBranchAddress("runnum",&runnum);
  tr->SetBranchAddress("Bunchid7bit",&bx);
  tr->SetBranchAddress("E12",&E12);
  tr->SetBranchAddress("Pt",&Pt);
  tr->SetBranchAddress("Eta",&Eta);
  tr->SetBranchAddress("Phi",&Phi);
  tr->SetBranchAddress("M12",&M12);
  tr->SetBranchAddress("Z",&Z);
  tr->SetBranchAddress("N12",&N12);
  tr->SetBranchAddress("L2sum",L2sum);
  tr->SetBranchAddress("lastdsm",trg_bool->TCU->lastdsm);

  tr->SetBranchAddress("RPE_QTN",&(trg_bool->RPSCI->N[kE]));
  tr->SetBranchAddress("RPW_QTN",&(trg_bool->RPSCI->N[kW]));
  tr->SetBranchAddress("RPE_Idx",trg_bool->RPSCI->Idx[kE]);
  tr->SetBranchAddress("RPE_TAC",trg_bool->RPSCI->TAC[kE]);
  tr->SetBranchAddress("RPE_ADC",trg_bool->RPSCI->ADC[kE]);
  tr->SetBranchAddress("RPW_Idx",trg_bool->RPSCI->Idx[kW]);
  tr->SetBranchAddress("RPW_TAC",trg_bool->RPSCI->TAC[kW]);
  tr->SetBranchAddress("RPW_ADC",trg_bool->RPSCI->ADC[kW]);
  tr->SetBranchAddress("RPvertex",trg_bool->RPSCI->vertex);


  // event classes
  Int_t NCLASS_tmp = ev->N;
  const Int_t NCLASS = NCLASS_tmp;

  // number of trigger booleans
  Int_t NBOOL_tmp = trg_bool->NBOOL;
  const Int_t NBOOL = NBOOL_tmp;
  Int_t unbiased_bool;
  for(i=0; i<NBOOL; i++)
  {
    if(!strcmp(trg_bool->Name(i),"N")) 
    {
      unbiased_bool=i;
      break;
    };
  };


  // obtain trigger boolean strength cases to analyse
  // -- case number = index for each case
  TTree * bool_case_tr = new TTree();
  bool_case_tr->ReadFile("boolean_cases.dat","stg1/I:stg2/I:mipn/I:use_tcu/I:case_name/C");
  Int_t stg1_r,stg2_r,mipn_r,use_tcu_r;
  char case_name_r[64];
  bool_case_tr->SetBranchAddress("stg1",&stg1_r);
  bool_case_tr->SetBranchAddress("stg2",&stg2_r);
  bool_case_tr->SetBranchAddress("mipn",&mipn_r);
  bool_case_tr->SetBranchAddress("use_tcu",&use_tcu_r);
  bool_case_tr->SetBranchAddress("case_name",case_name_r);
  Int_t NCASE_tmp = bool_case_tr->GetEntries();
  const Int_t NCASE = NCASE_tmp;
  Int_t STG1_case[NCASE]; // [case number]
  Int_t STG2_case[NCASE];
  Int_t MIPN_case[NCASE];
  Int_t USE_TCU_BITS_case[NCASE];
  TString str_case[NCASE];
  TString case_name[NCASE];
  for(i=0; i<NCASE; i++)
  {
    bool_case_tr->GetEntry(i);
    STG1_case[i] = stg1_r;
    STG2_case[i] = stg2_r;
    MIPN_case[i] = mipn_r;
    USE_TCU_BITS_case[i] = use_tcu_r;
    case_name[i] = Form("%s",case_name_r);
    str_case[i] = Form("%d%d%d%d",STG1_case[i],STG2_case[i],MIPN_case[i],USE_TCU_BITS_case[i]);
  };

  // determine combinations of trigger boolean strength cases to analyse overlap
  // -- combo number = index of pair of cases to compare overlap
  // -- each pair has cases denoted as "left" and "right"
  Int_t NCOMBO_tmp = NCASE*(NCASE-1); // NCASE choose 2
  const Int_t NCOMBO = NCOMBO_tmp;
  Int_t STG1_combo[NCOMBO][2]; // [combo number] [0=left / 1=right]
  Int_t STG2_combo[NCOMBO][2];
  Int_t MIPN_combo[NCOMBO][2];
  Int_t USE_TCU_BITS_combo[NCOMBO][2];
  TString str_combo[NCOMBO][2];
  TString combo_name[NCOMBO][2];
  Int_t case_combo[NCOMBO][2];
  Int_t c=0;
  for(Int_t l=0; l<NCASE; l++)
  {
    for(Int_t r=0; r<NCASE; r++)
    {
      if(l!=r)
      {
        printf("%d %d %d\n",c,l,r);

        STG1_combo[c][kL] = STG1_case[l];
        STG2_combo[c][kL] = STG2_case[l];
        MIPN_combo[c][kL] = MIPN_case[l];
        USE_TCU_BITS_combo[c][kL] = USE_TCU_BITS_case[l];
        str_combo[c][kL] = 
          Form("%d%d%d%d",STG1_combo[c][kL],STG2_combo[c][kL],MIPN_combo[c][kL],USE_TCU_BITS_combo[c][kL]);
        combo_name[c][kL] = case_name[l];
        case_combo[c][kL] = l;
        
        STG1_combo[c][kR] = STG1_case[r];
        STG2_combo[c][kR] = STG2_case[r];
        MIPN_combo[c][kR] = MIPN_case[r];
        USE_TCU_BITS_combo[c][kR] = USE_TCU_BITS_case[r];
        str_combo[c][kR] = 
          Form("%d%d%d%d",STG1_combo[c][kR],STG2_combo[c][kR],MIPN_combo[c][kR],USE_TCU_BITS_combo[c][kR]);
        combo_name[c][kR] = case_name[r];
        case_combo[c][kR] = r;

        c++;
      };
    };
  };


  // define overlap matrices (they get initialised every time new run detected in tree loop)
  TH2D * bool_w_case[MAX_RUNS]; // trigger boolean vs. case
  TString bool_w_case_n[MAX_RUNS];
  TString bool_w_case_t[MAX_RUNS];

  TH2D * bool_prob_case[MAX_RUNS]; // probability of boolean firing per bXing
  TString bool_prob_case_n[MAX_RUNS];
  TString bool_prob_case_t[MAX_RUNS];

  TH2D * bool_w_bool[NCOMBO][MAX_RUNS]; // trigger boolean "left case" vs. trigger boolean "right case"
  TString bool_w_bool_n[NCOMBO][MAX_RUNS];
  TString bool_w_bool_t[NCOMBO][MAX_RUNS];

  TH2D * bool_frac_bool[NCOMBO][MAX_RUNS]; // fraction of "left" which satisfy "right"
  TString bool_frac_bool_n[NCOMBO][MAX_RUNS];
  TString bool_frac_bool_t[NCOMBO][MAX_RUNS];


  // tree loop
  Bool_t trg[NBOOL][NCASE];
  Int_t runnum_tmp=0;
  Int_t runnum_list[MAX_RUNS];
  Int_t run_count=-1;
  memset(runnum_list,0,sizeof(runnum_list));
  Int_t ENT = tr->GetEntries();
  //ENT=10;
  for(Int_t x=0; x<ENT; x++)
  {
    if((x%100000)==0) printf("filling histograms: %.2f%%\n",100*((Float_t)x)/((Float_t)ENT));
    tr->GetEntry(x);

    // new run num detected:
    if(runnum!=runnum_tmp)
    {
      runnum_tmp = runnum;

      b_pol = RD->BluePol(runnum);
      y_pol = RD->YellPol(runnum);
      isConsistent = RD->RellumConsistent(runnum);

      // define new matrices if the run is usable
      if(isConsistent==1 && b_pol>0 && y_pol>0)
      {
        run_count++;

        bool_w_case_n[run_count] = Form("bool_w_case_r%d",runnum);
        bool_w_case_t[run_count] = Form("boolean vs. strength cases -- r%d",runnum);
        bool_w_case[run_count] = 
          new TH2D(bool_w_case_n[run_count].Data(),bool_w_case_t[run_count].Data(),NCASE,0,NCASE,NBOOL,0,NBOOL);

        bool_prob_case_n[run_count] = Form("bool_prob_case_r%d",runnum);
        bool_prob_case_t[run_count] = Form("boolean probabilities for each case -- r%d",runnum);
        bool_prob_case[run_count] = 
          new TH2D(bool_prob_case_n[run_count].Data(),bool_prob_case_t[run_count].Data(),NCASE,0,NCASE,NBOOL,0,NBOOL);

        for(int k=0; k<NCASE; k++) 
        {
          bool_w_case[run_count]->GetXaxis()->SetBinLabel(k+1,case_name[k].Data());
          bool_prob_case[run_count]->GetXaxis()->SetBinLabel(k+1,case_name[k].Data());
        };
        for(int k=0; k<NBOOL; k++) 
        {
          bool_w_case[run_count]->GetYaxis()->SetBinLabel(k+1,trg_bool->Name(k));
          bool_prob_case[run_count]->GetYaxis()->SetBinLabel(k+1,trg_bool->Name(k));
        };

        for(j=0; j<NCOMBO; j++)
        {
          bool_w_bool_n[j][run_count] = Form("%s_w_%s_r%d",combo_name[j][kL].Data(),combo_name[j][kR].Data(),runnum);
          bool_w_bool_t[j][run_count] = Form("case %s vs. case %s -- r%d",combo_name[j][kL].Data(),combo_name[j][kR].Data(),runnum);
          bool_w_bool[j][run_count] = 
            new TH2D(bool_w_bool_n[j][run_count].Data(),bool_w_bool_t[j][run_count].Data(),NBOOL,0,NBOOL,NBOOL,0,NBOOL);

          bool_frac_bool_n[j][run_count] = Form("%s_frac_%s_r%d",combo_name[j][kL].Data(),combo_name[j][kR].Data(),runnum);
          bool_frac_bool_t[j][run_count] = Form("fraction of case %s boolean which satisfy case %s boolean -- r%d",
            combo_name[j][kL].Data(),combo_name[j][kR].Data(),runnum);
          bool_frac_bool[j][run_count] = 
            new TH2D(bool_frac_bool_n[j][run_count].Data(),bool_frac_bool_t[j][run_count].Data(),NBOOL,0,NBOOL,NBOOL,0,NBOOL);

          for(int k=0; k<NBOOL; k++) 
          {
            bool_w_bool[j][run_count]->GetXaxis()->SetBinLabel(k+1,trg_bool->Name(k));
            bool_w_bool[j][run_count]->GetYaxis()->SetBinLabel(k+1,trg_bool->Name(k));
            bool_frac_bool[j][run_count]->GetXaxis()->SetBinLabel(k+1,trg_bool->Name(k));
            bool_frac_bool[j][run_count]->GetYaxis()->SetBinLabel(k+1,trg_bool->Name(k));
          };
        };
      };
    };

    kicked = RD->Kicked(runnum,bx);

    // if rellum and polarisation ok and if it's an FMS trigger, we check the event class
    if(isConsistent==1 && b_pol>0 && y_pol>0 && kicked==0 && (L2sum[1] & T->Mask(runnum,l2trg,1)))
    {
      ev->SetKinematics(runnum,E12,Pt,Eta,Phi,M12,Z,N12);
      // if the event is valid, then we check the trigger booleans
      if(ev->Valid(evc))
      {
        // reset trigger bits
        for(Int_t cc=0; cc<NCASE; cc++)
        {
          for(Int_t bb=0; bb<NBOOL; bb++)
          {
            trg[bb][cc] = false;
          };
        };

        // now check if trigger bits fired for each boolean & case
        for(Int_t cc=0; cc<NCASE; cc++)
        {
          for(Int_t bb=0; bb<NBOOL; bb++)
          {
            trg[bb][cc] = trg_bool->FiredAlternate(bb,STG1_case[cc],STG2_case[cc],MIPN_case[cc],USE_TCU_BITS_case[cc]);
            if(trg[bb][cc]) bool_w_case[run_count]->Fill(cc,bb);
          };
        };

        // fill overlap matrices
        for(Int_t mm=0; mm<NCOMBO; mm++)
        {
          for(Int_t bbl=0; bbl<NBOOL; bbl++)
          {
            for(Int_t bbr=0; bbr<NBOOL; bbr++)
            {
              if( trg[bbl][case_combo[mm][kL]] && 
                  trg[bbr][case_combo[mm][kR]] ) 
                bool_w_bool[mm][run_count]->Fill(bbr,bbl);
            };
          };
        };
      }; // eo if ev->Valid
    }; // eo if isConsistent
  }; // eo tree loop


  // compute probability of boolean firing for each case
  Double_t denom,numer;
  Int_t bn;
  for(int r=0; r<=run_count; r++)
  {
    for(int cc=0; cc<NBOOL; cc++)
    {
      bn = bool_w_case[r]->GetBin(cc+1,1);
      denom = bool_w_case[r]->GetBinContent(bn);
      for(int b=0; b<NBOOL; b++)
      {
        bn = bool_w_case[r]->GetBin(cc+1,b+1);
        numer = bool_w_case[r]->GetBinContent(bn);
        bool_prob_case[r]->SetBinContent(bn,numer/denom);
      };
    };
  };


  // compute fraction of "left" case boolean which satisfy "right" case boolean
  for(int r=0; r<=run_count; r++)
  {
    for(i=0; i<NCOMBO; i++)
    {
      for(int b=0; b<NBOOL; b++)
      {
        bn = bool_w_bool[i][r]->GetBin(unbiased_bool+1,b+1);
        denom = bool_w_bool[i][r]->GetBinContent(bn);
        for(int bb=0; bb<NBOOL; bb++)
        {
          bn = bool_w_bool[i][r]->GetBin(bb+1,b+1);
          numer = bool_w_bool[i][r]->GetBinContent(bn);
          bool_frac_bool[i][r]->SetBinContent(bn,numer/denom);
        };
      };
    };
  };


  // write output
  TString outfile_name = Form("overlapset/OverlapSet%s.root",set_name);
  TFile * outifle = new TFile(outfile_name.Data(),"RECREATE");
  TObjArray * bool_w_case_arr = new TObjArray();
  TObjArray * bool_prob_case_arr = new TObjArray();
  TObjArray * bool_w_bool_arr[NCOMBO];
  TObjArray * bool_frac_bool_arr[NCOMBO];
  TString bool_w_case_arr_n = "bool_w_case_arr";
  TString bool_prob_case_arr_n = "bool_prob_case_arr";
  TString bool_w_bool_arr_n[NCOMBO];
  TString bool_frac_bool_arr_n[NCOMBO];
  for(j=0; j<NCOMBO; j++) 
  {
    bool_w_bool_arr[j] =  new TObjArray();
    bool_frac_bool_arr[j] =  new TObjArray();
  };
  for(i=0; i<=run_count; i++)
  {
    bool_w_case_arr->Add(bool_w_case[i]);
    bool_prob_case_arr->Add(bool_prob_case[i]);
    for(j=0; j<NCOMBO; j++) 
    {
      bool_w_bool_arr[j]->Add(bool_w_bool[j][i]);
      bool_frac_bool_arr[j]->Add(bool_frac_bool[j][i]);
    };
  };
  bool_w_case_arr->Write(bool_w_case_arr_n.Data(),TObject::kSingleKey);
  bool_prob_case_arr->Write(bool_prob_case_arr_n.Data(),TObject::kSingleKey);
  for(j=0; j<NCOMBO; j++) 
  {
    bool_w_bool_arr_n[j] = Form("%s_w_%s_arr",combo_name[j][kL].Data(),combo_name[j][kR].Data());
    bool_w_bool_arr[j]->Write(bool_w_bool_arr_n[j].Data(),TObject::kSingleKey);
  };
  for(j=0; j<NCOMBO; j++) 
  {
    bool_frac_bool_arr_n[j] = Form("%s_frac_%s_arr",combo_name[j][kL].Data(),combo_name[j][kR].Data());
    bool_frac_bool_arr[j]->Write(bool_frac_bool_arr_n[j].Data(),TObject::kSingleKey);
  };
};
