// -- create phi distributions with various kinematic cuts (eta,pt,E) for each spinbit
//    (kinematic cuts are set as Double_t's below)
// -- phi distributions for "+ -" and "- +" are weighted by relative luminosity
//    A_LL = 1/(P_b*P_y)*(N++ + N-- - RN+- -RN-+)/(N++ + N-- + RN+- + RN-+)
// -- phi distributions are written to phiset/ directory with similar name; 
//    they are named: phi_s[spinbit]_g[eta bin]_p[pt bin]_e[en bin]

void TestRPevents(const char * filename="RedOutputset080ac.root")
{
  enum ew_enum {kE,kW};
  enum ud_enum {kU,kD};
  enum io_enum {kI,kO};
  enum ns_enum {kN,kS};

  // load polarization and rellum data
  gSystem->Load("src/RunInfo.so");
  RunInfo * RD = new RunInfo();
  LevelTwo * T = new LevelTwo();
  Environ * env = new Environ();
  EventClass * ev = new EventClass();
  TriggerBoolean * trg_bool = 
    new TriggerBoolean(env->STG1,env->STG2,env->MIPN,env->USE_TCU_BITS);
  trg_bool->PrintParameters();

  // get bins from environment
  Int_t phi_bins0 = env->PhiBins; const Int_t phi_bins = phi_bins0;
  Int_t eta_bins0 = env->EtaBins; const Int_t eta_bins = eta_bins0;
  Int_t en_bins0 = env->EnBins; const Int_t en_bins = en_bins0;
  Int_t pt_bins0 = env->PtBins; const Int_t pt_bins = pt_bins0;

  // get number of event classes
  Int_t N_CLASS_tmp = ev->N;
  const Int_t N_CLASS = N_CLASS_tmp;


  // read redset tree and set output file name
  Int_t runnum,bx,blue,yell,pattern;
  Float_t M12,N12,E12,Z,Phi,Eta,Pt,b_pol,y_pol;
  Bool_t kicked,isConsistent;
  UInt_t L2sum[2];
  UInt_t lastdsm[8];
  char setname[32];
  sscanf(filename,"RedOutputset%s",setname);
  sprintf(filename,"redset/%s",filename);
  TFile * infile = new TFile(filename,"READ");
  char outname[256];
  sprintf(outname,"phiset/phi%s",setname);
  TTree * tree = (TTree*) infile->Get("str");
  tree->SetBranchAddress("runnum",&runnum);
  tree->SetBranchAddress("Bunchid7bit",&bx);
  tree->SetBranchAddress("M12",&M12);
  tree->SetBranchAddress("N12",&N12);
  tree->SetBranchAddress("E12",&E12);
  tree->SetBranchAddress("Z",&Z);
  tree->SetBranchAddress("Phi",&Phi);
  tree->SetBranchAddress("Eta",&Eta);
  tree->SetBranchAddress("Pt",&Pt);
  tree->SetBranchAddress("L2sum",L2sum);
  tree->SetBranchAddress("lastdsm",trg_bool->TCU->lastdsm);

  tree->SetBranchAddress("RPE_QTN",&(trg_bool->RPSCI->N[kE]));
  tree->SetBranchAddress("RPW_QTN",&(trg_bool->RPSCI->N[kW]));
  tree->SetBranchAddress("RPE_Idx",trg_bool->RPSCI->Idx[kE]);
  tree->SetBranchAddress("RPE_TAC",trg_bool->RPSCI->TAC[kE]);
  tree->SetBranchAddress("RPE_ADC",trg_bool->RPSCI->ADC[kE]);
  tree->SetBranchAddress("RPW_Idx",trg_bool->RPSCI->Idx[kW]);
  tree->SetBranchAddress("RPW_TAC",trg_bool->RPSCI->TAC[kW]);
  tree->SetBranchAddress("RPW_ADC",trg_bool->RPSCI->ADC[kW]);
  tree->SetBranchAddress("RPvertex",&(trg_bool->RPSCI->vertex));


  // define spinbit strings
  char spinbit_t[4][4];
  char spinbit_n[4][4];
  strcpy(spinbit_t[0],"--"); strcpy(spinbit_n[0],"nn");
  strcpy(spinbit_t[1],"-+"); strcpy(spinbit_n[1],"np");
  strcpy(spinbit_t[2],"+-"); strcpy(spinbit_n[2],"pn");
  strcpy(spinbit_t[3],"++"); strcpy(spinbit_n[3],"pp");



  // tree loop

  Bool_t fff;
  for(Int_t x=0; x<tree->GetEntries(); x++)
  {
    if((x%10000)==0) printf("%.2f%%\n",100*((Float_t)x)/((Float_t)tree->GetEntries()));

    trg_bool->RPSCI->ResetBranches();
    trg_bool->RPSCI->ResetBits();
    tree->GetEntry(x);

    ev->SetKinematics(runnum,E12,Pt,Eta,Phi,M12,Z,N12);
    if(ev->Valid(ev->Idx("pi0")))
    {
      fff = trg_bool->Fired(env->RPselect);
      gSystem->RedirectOutput("debug.dat"); trg_bool->Diagnostic(runnum,x); gSystem->RedirectOutput(0);
      //printf("EVENT %d: %s fired=%d FMS=%d\n",x,env->RPselect,trg_bool->Fired(env->RPselect),L2sum[1]&T->Mask(runnum,env->TriggerType,1));
      //printf("-----------\n\n");
    };
  };
};
