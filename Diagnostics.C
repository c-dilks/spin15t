// shows the dependence of various kinematics on geometry
// e.g., pt vs. eta
// --> outputs all plots to diag_[trigger].pdf

void Diagnostics()
{
  const Int_t NBINS=400; // NUMBER OF BINS

  gSystem->Load("src/RunData.so");
  RunData * RD = new RunData();
  Trigger * T = new Trigger();
  Environ * env = new Environ();


  // open chain
  TChain * tr = new TChain("str");
  tr->Add("./redset/Red*.root");
  Float_t E12,Pt,Eta,Phi,M12,Z,b_pol,y_pol;
  Bool_t kicked,isConsistent;
  Int_t runnum,bx;
  Int_t L2sum[2];
  Float_t N12;

  Float_t E12_min,Pt_min,Eta_min,Phi_min;
  Float_t E12_max,Pt_max,Eta_max,Phi_max;
  E12_min=Pt_min=Eta_min=Phi_min=1000;
  E12_max=Pt_max=Eta_max=Phi_max=0;

  str->SetBranchAddress("runnum",&runnum);
  str->SetBranchAddress("Bunchid7bit",&bx);
  str->SetBranchAddress("E12",&E12);
  str->SetBranchAddress("Pt",&Pt);
  str->SetBranchAddress("Eta",&Eta);
  str->SetBranchAddress("Phi",&Phi);
  str->SetBranchAddress("M12",&M12);
  str->SetBranchAddress("Z",&Z);
  str->SetBranchAddress("N12",&N12);
  str->SetBranchAddress("L2sum",L2sum);
  Int_t runnum_tmp=0;

  // trigger list
  Int_t N_TRIG_tmp = T->N;
  const Int_t N_TRIG = N_TRIG_tmp;

  // event classes
  const Int_t N_CLASS = 3;
  enum class_enum {kSph,kPi0,kThr};
  char class_name[N_CLASS][8];
  char class_title[N_CLASS][32];
  strcpy(class_name[kSph],"sph");
  strcpy(class_name[kPi0],"pi0");
  strcpy(class_name[kThr],"thr");
  strcpy(class_title[kSph],"single #gamma");
  strcpy(class_title[kPi0],"#pi^{0} (naive M cut)");
  strcpy(class_title[kThr],"N_{#gamma}>2");


  // get kinematic ranges using the current binning set by Bin_Splitter.C
  Phi_min = env->PhiLow;
  Phi_max = env->PhiHigh;
  Eta_min = env->EtaLow;
  Eta_max = env->EtaHigh;
  E12_min = env->EnLow;
  E12_max = env->EnHigh;
  Pt_min = env->PtLow;
  Pt_max = env->PtHigh;

  
  // load run exclusion trees
  TTree * exclusion_sph = new TTree("exclusion_sph","exclusion_sph");
  TTree * exclusion_pi0 = new TTree("exclusion_pi0","exclusion_pi0");
  TTree * exclusion_thr = new TTree("exclusion_thr","exclusion_thr");
  exclusion_sph->ReadFile("exclusion_list_sph","runnum/I");
  exclusion_pi0->ReadFile("exclusion_list_pi0","runnum/I");
  exclusion_thr->ReadFile("exclusion_list_thr","runnum/I");
  Bool_t exclude_sph,exclude_pi0,exclude_thr;
  Int_t rn_sph,rn_pi0,rn_thr;
  exclusion_sph->SetBranchAddress("runnum",&rn_sph);
  exclusion_pi0->SetBranchAddress("runnum",&rn_pi0);
  exclusion_thr->SetBranchAddress("runnum",&rn_thr);

  
  // define histograms
  TH2D * pt_vs_eta[N_CLASS][N_TRIG];
  TH2D * en_vs_eta[N_CLASS][N_TRIG];
  TH2D * pt_vs_phi[N_CLASS][N_TRIG];
  TH2D * en_vs_phi[N_CLASS][N_TRIG];
  TH2D * eta_vs_phi[N_CLASS][N_TRIG];
  TH2D * pt_vs_en[N_CLASS][N_TRIG];

  TH2D * pi0_z_vs_eta[N_TRIG];
  TH2D * pi0_z_vs_phi[N_TRIG];
  TH2D * pi0_mass_vs_en[N_TRIG];
  TH2D * pi0_mass_vs_pt[N_TRIG];
  TH1D * pi0_mass_dist[N_TRIG];
  TH1D * pi0_z_dist[N_TRIG];

  char pt_vs_eta_n[N_CLASS][N_TRIG][32];
  char en_vs_eta_n[N_CLASS][N_TRIG][32];
  char pt_vs_phi_n[N_CLASS][N_TRIG][32];
  char en_vs_phi_n[N_CLASS][N_TRIG][32];
  char eta_vs_phi_n[N_CLASS][N_TRIG][32];
  char pt_vs_en_n[N_CLASS][N_TRIG][32];
  char pi0_z_vs_eta_n[N_TRIG][32];
  char pi0_z_vs_phi_n[N_TRIG][32];
  char pi0_mass_vs_en_n[N_TRIG][32];
  char pi0_mass_vs_pt_n[N_TRIG][32];
  char pi0_mass_dist_n[N_TRIG][32];
  char pi0_z_dist_n[N_TRIG][32];

  char pt_vs_eta_t[N_CLASS][N_TRIG][128];
  char en_vs_eta_t[N_CLASS][N_TRIG][128];
  char pt_vs_phi_t[N_CLASS][N_TRIG][128];
  char en_vs_phi_t[N_CLASS][N_TRIG][128];
  char eta_vs_phi_t[N_CLASS][N_TRIG][128];
  char pt_vs_en_t[N_CLASS][N_TRIG][128];
  char pi0_z_vs_eta_t[N_TRIG][128];
  char pi0_z_vs_phi_t[N_TRIG][128];
  char pi0_mass_vs_en_t[N_TRIG][128];
  char pi0_mass_vs_pt_t[N_TRIG][128];
  char pi0_mass_dist_t[N_TRIG][128];
  char pi0_z_dist_t[N_TRIG][128];

  for(Int_t t=0; t<N_TRIG; t++)
  {
    for(Int_t c=0; c<N_CLASS; c++)
    {
      sprintf(pt_vs_eta_n[c][t],"%s_%s_pt_vs_eta",T->Name(t),class_name[c]);
      sprintf(en_vs_eta_n[c][t],"%s_%s_en_vs_eta",T->Name(t),class_name[c]);
      sprintf(pt_vs_phi_n[c][t],"%s_%s_pt_vs_phi",T->Name(t),class_name[c]);
      sprintf(en_vs_phi_n[c][t],"%s_%s_en_vs_phi",T->Name(t),class_name[c]);
      sprintf(eta_vs_phi_n[c][t],"%s_%s_eta_vs_phi",T->Name(t),class_name[c]);
      sprintf(pt_vs_en_n[c][t],"%s_%s_pt_vs_en",T->Name(t),class_name[c]);

      sprintf(pt_vs_eta_t[c][t],"%s %s :: p_{T} vs. #eta;#eta;p_{T}",T->Name(t),class_title[c]);
      sprintf(en_vs_eta_t[c][t],"%s %s :: E vs. #eta;#eta;E",T->Name(t),class_title[c]);
      sprintf(pt_vs_phi_t[c][t],"%s %s :: p_{T} vs. #phi;#phi;p_{T}",T->Name(t),class_title[c]);
      sprintf(en_vs_phi_t[c][t],"%s %s :: E vs. #phi;#phi;E",T->Name(t),class_title[c]);
      sprintf(eta_vs_phi_t[c][t],"%s %s :: #eta vs. #phi;#phi;#eta",T->Name(t),class_title[c]);
      sprintf(pt_vs_en_t[c][t],"%s %s :: p_{T} vs. E;E;p_{T}",T->Name(t),class_title[c]);

      pt_vs_eta[c][t] = new TH2D(pt_vs_eta_n[c][t],pt_vs_eta_t[c][t],NBINS,Eta_min,Eta_max,NBINS,Pt_min,Pt_max);
      en_vs_eta[c][t] = new TH2D(en_vs_eta_n[c][t],en_vs_eta_t[c][t],NBINS,Eta_min,Eta_max,NBINS,E12_min,E12_max);
      pt_vs_phi[c][t] = new TH2D(pt_vs_phi_n[c][t],pt_vs_phi_t[c][t],NBINS,Phi_min,Phi_max,NBINS,Pt_min,Pt_max);
      en_vs_phi[c][t] = new TH2D(en_vs_phi_n[c][t],en_vs_phi_t[c][t],NBINS,Phi_min,Phi_max,NBINS,E12_min,E12_max);
      eta_vs_phi[c][t] = new TH2D(eta_vs_phi_n[c][t],eta_vs_phi_t[c][t],NBINS,Phi_min,Phi_max,NBINS,Eta_min,Eta_max);
      pt_vs_en[c][t] = new TH2D(pt_vs_en_n[c][t],pt_vs_en_t[c][t],NBINS,E12_min,E12_max,NBINS,Pt_min,Pt_max);
    };

    sprintf(pi0_z_vs_eta_n[t],"%s_%s_z_vs_eta",T->Name(t),class_name[kPi0]);
    sprintf(pi0_z_vs_phi_n[t],"%s_%s_z_vs_phi",T->Name(t),class_name[kPi0]);
    sprintf(pi0_mass_vs_en_n[t],"%s_%s_mass_vs_en",T->Name(t),class_name[kPi0]);
    sprintf(pi0_mass_vs_pt_n[t],"%s_%s_mass_vs_pt",T->Name(t),class_name[kPi0]);
    sprintf(pi0_mass_dist_n[t],"%s_%s_mass_dist",T->Name(t),class_name[kPi0]);
    sprintf(pi0_z_dist_n[t],"%s_%s_z_dist",T->Name(t),class_name[kPi0]);

    sprintf(pi0_z_vs_eta_t[t],"%s %s :: Z vs. #eta;#eta;Z",T->Name(t),class_name[kPi0]);
    sprintf(pi0_z_vs_phi_t[t],"%s %s :: Z vs. #phi;#phi;Z",T->Name(t),class_name[kPi0]);
    sprintf(pi0_mass_vs_en_t[t],"%s %s :: M vs. E (#pi^{0} cuts w/o M-cut);E;M",T->Name(t),class_name[kPi0]);
    sprintf(pi0_mass_vs_pt_t[t],"%s %s :: M vs. p_{T} (#pi^{0} cuts w/o M-cut);p_{T};M",T->Name(t),class_name[kPi0]);
    sprintf(pi0_mass_dist_t[t],"%s %s :: M distribution (#pi^{0} cuts w/o M-cut);M",T->Name(t),class_name[kPi0]);
    sprintf(pi0_z_dist_t[t],"%s %s :: Z distribution (naive mass cut only);Z",T->Name(t),class_name[kPi0]);

    pi0_z_vs_eta[t] = new TH2D(pi0_z_vs_eta_n[t],pi0_z_vs_eta_t[t],NBINS,Eta_min,Eta_max,NBINS,0,1);
    pi0_z_vs_phi[t] = new TH2D(pi0_z_vs_phi_n[t],pi0_z_vs_phi_t[t],NBINS,Phi_min,Phi_max,NBINS,0,1);
    pi0_mass_vs_en[t] = new TH2D(pi0_mass_vs_en_n[t],pi0_mass_vs_en_t[t],NBINS,E12_min,E12_max,NBINS,0,1);
    pi0_mass_vs_pt[t] = new TH2D(pi0_mass_vs_pt_n[t],pi0_mass_vs_pt_t[t],NBINS,Pt_min,Pt_max,NBINS,0,1);
    pi0_mass_dist[t] = new TH1D(pi0_mass_dist_n[t],pi0_mass_dist_t[t],NBINS,0,1);
    pi0_z_dist[t] = new TH1D(pi0_z_dist_n[t],pi0_z_dist_t[t],NBINS,0,1);
  };


  // energy-dependent mass distributions
  TH1D * mass_dist_for_enbin[N_TRIG][10];
  char mass_dist_for_enbin_n[N_TRIG][10][64];
  char mass_dist_for_enbin_t[N_TRIG][10][256];
  for(Int_t t=0; t<N_TRIG; t++)
  {
    for(Int_t ee=0; ee<10; ee++)
    {
      sprintf(mass_dist_for_enbin_n[t][ee],"%s_mass_dist_for_enbin%d",T->Name(t),ee);
      sprintf(mass_dist_for_enbin_t[t][ee],
        "%s M_{#gamma#gamma} distribution for E_{#gamma#gamma}#in[%d,%d) GeV",
        T->Name(t),ee*10,(ee+1)*10);
      mass_dist_for_enbin[t][ee] = new TH1D(mass_dist_for_enbin_n[t][ee],mass_dist_for_enbin_t[t][ee],NBINS,0,1);
    };
  };

  // pt-dependent mass distributions
  TH1D * mass_dist_for_ptbin[N_TRIG][10];
  char mass_dist_for_ptbin_n[N_TRIG][10][64];
  char mass_dist_for_ptbin_t[N_TRIG][10][256];
  for(Int_t t=0; t<N_TRIG; t++)
  {
    for(Int_t pp=0; pp<10; pp++)
    {
      sprintf(mass_dist_for_ptbin_n[t][pp],"%s_mass_dist_for_ptbin%d",T->Name(t),pp);
      sprintf(mass_dist_for_ptbin_t[t][pp],
        "%s M_{#gamma#gamma} distribution for p_{T}#in[%d,%d) GeV/c",
        T->Name(t),pp,pp+1);
      mass_dist_for_ptbin[t][pp] = new TH1D(mass_dist_for_ptbin_n[t][pp],mass_dist_for_ptbin_t[t][pp],NBINS,0,1);
    };
  };

  // trigger distribution
  TH1D * trig_dist = new TH1D("trig_dist","Trigger Counts",N_TRIG,0,N_TRIG);
  for(Int_t t=0; t<N_TRIG; t++) trig_dist->GetXaxis()->SetBinLabel(t+1,T->Name(t));



  // fill histograms
  runnum_tmp=0;
  for(Int_t x=0; x<tr->GetEntries(); x++)
  {
    if((x%100000)==0) printf("filling histograms: %.2f%%\n",100*((Float_t)x)/((Float_t)tr->GetEntries()));
    tr->GetEntry(x);
    kicked = RD->Kicked(runnum,bx);
    if(runnum!=runnum_tmp)
    {
      b_pol = RD->BluePol(runnum);
      y_pol = RD->YellPol(runnum);
      isConsistent = RD->RellumConsistent(runnum);
      runnum_tmp=runnum;
      exclude_sph=0;
      exclude_pi0=0;
      exclude_thr=0;
      for(Int_t xx=0; xx<exclusion_sph->GetEntries(); xx++) { exclusion_sph->GetEntry(xx); if(runnum==rn_sph) exclude_sph=1; };
      for(Int_t xx=0; xx<exclusion_pi0->GetEntries(); xx++) { exclusion_pi0->GetEntry(xx); if(runnum==rn_pi0) exclude_pi0=1; };
      for(Int_t xx=0; xx<exclusion_thr->GetEntries(); xx++) { exclusion_thr->GetEntry(xx); if(runnum==rn_thr) exclude_thr=1; };
    }
    // rellum / pol cut
    if( kicked==0 && isConsistent==1 && b_pol>0 && y_pol>0)
    {
      // fill trigger plot
      for(Int_t t=0; t<N_TRIG; t++)
      {
        if(L2sum[1] & T->Mask(runnum,t,1)) trig_dist->Fill(t);
      };

      // below here we have if statements for each event class; within each
      // if block, we have trigger loops, since it's more efficient to first pick the event and 
      // then loop through all triggers

      // fill pi0 plots
      // IF YOU CHANGE THE CUTS HERE, CHANGE THEM IN THE PLOT TITLES TOO!!!!!
      // -- mass and z-cuts are given in if statements below!
      if(exclude_pi0==0 && 
         fabs(N12-2)<0.01 &&
         Pt>Pt_min && Pt<Pt_max &&
         E12>=E12_min && E12<E12_max &&
         M12>0) 
      {
        for(Int_t t=0; t<N_TRIG; t++)
        {
          if(L2sum[1] & T->Mask(runnum,t,1))
          {
            if(Z < 0.8)
            {
              pi0_mass_dist[t]->Fill(M12);
              pi0_mass_vs_en[t]->Fill(E12,M12);
              pi0_mass_vs_pt[t]->Fill(Pt,M12);
              for(Int_t ee=0; ee<10; ee++)
              {
                if(E12>=(ee*10) && E12<((ee+1)*10)) mass_dist_for_enbin[t][ee]->Fill(M12);
              };
              for(Int_t pp=0; pp<10; pp++)
              {
                if(Pt>=pp && Pt<(pp+1)) mass_dist_for_ptbin[t][pp]->Fill(M12);
              };
              // naive mass cut for correlation plots
              if(fabs(M12-0.135)<0.1)
              {
                pt_vs_eta[kPi0][t]->Fill(Eta,Pt);
                en_vs_eta[kPi0][t]->Fill(Eta,E12);
                pt_vs_phi[kPi0][t]->Fill(Phi,Pt);
                en_vs_phi[kPi0][t]->Fill(Phi,E12);
                eta_vs_phi[kPi0][t]->Fill(Phi,Eta);
                pt_vs_en[kPi0][t]->Fill(E12,Pt);
                pi0_z_vs_eta[t]->Fill(Eta,Z);
                pi0_z_vs_phi[t]->Fill(Phi,Z);
              };
            };
            if(fabs(M12-0.135)<0.1) pi0_z_dist[t]->Fill(Z);
          };
        };
      };

      // fill single photon plots
      if(exclude_sph==0 && fabs(N12-1)<0.01)
      {
        for(Int_t t=0; t<N_TRIG; t++)
        {
          if(L2sum[1] & T->Mask(runnum,t,1))
          {
            pt_vs_eta[kSph][t]->Fill(Eta,Pt);
            en_vs_eta[kSph][t]->Fill(Eta,E12);
            pt_vs_phi[kSph][t]->Fill(Phi,Pt);
            en_vs_phi[kSph][t]->Fill(Phi,E12);
            eta_vs_phi[kSph][t]->Fill(Phi,Eta);
            pt_vs_en[kSph][t]->Fill(E12,Pt);
          };
        };
      };

      // three or more photons cut
      if(exclude_thr==0 && N12>2.5)
      {
        for(Int_t t=0; t<N_TRIG; t++)
        {
          if(L2sum[1] & T->Mask(runnum,t,1))
          {
            pt_vs_eta[kThr][t]->Fill(Eta,Pt);
            en_vs_eta[kThr][t]->Fill(Eta,E12);
            pt_vs_phi[kThr][t]->Fill(Phi,Pt);
            en_vs_phi[kThr][t]->Fill(Phi,E12);
            eta_vs_phi[kThr][t]->Fill(Phi,Eta);
            pt_vs_en[kThr][t]->Fill(E12,Pt);
          };
        };
      };
    };
  };

  
  // build TObjArrays
  TObjArray * pt_vs_eta_arr[N_CLASS];
  TObjArray * en_vs_eta_arr[N_CLASS];
  TObjArray * pt_vs_phi_arr[N_CLASS];
  TObjArray * en_vs_phi_arr[N_CLASS];
  TObjArray * eta_vs_phi_arr[N_CLASS];
  TObjArray * pt_vs_en_arr[N_CLASS];
  TObjArray * pi0_z_vs_eta_arr = new TObjArray();
  TObjArray * pi0_z_vs_phi_arr = new TObjArray();
  TObjArray * pi0_mass_vs_en_arr = new TObjArray();
  TObjArray * pi0_mass_vs_pt_arr = new TObjArray();
  TObjArray * pi0_mass_dist_arr = new TObjArray();
  TObjArray * pi0_z_dist_arr = new TObjArray();
  TObjArray * mass_dist_for_enbin_arr[10];
  TObjArray * mass_dist_for_ptbin_arr[10];

  char pt_vs_eta_arr_n[N_CLASS][32];
  char en_vs_eta_arr_n[N_CLASS][32];
  char pt_vs_phi_arr_n[N_CLASS][32];
  char en_vs_phi_arr_n[N_CLASS][32];
  char eta_vs_phi_arr_n[N_CLASS][32];
  char pt_vs_en_arr_n[N_CLASS][32];
  char pi0_z_vs_eta_arr_n[32];
  char pi0_z_vs_phi_arr_n[32];
  char pi0_mass_vs_en_arr_n[32];
  char pi0_mass_vs_pt_arr_n[32];
  char pi0_mass_dist_arr_n[32];
  char pi0_z_dist_arr_n[32];
  char mass_dist_for_enbin_arr_n[10][32];
  char mass_dist_for_ptbin_arr_n[10][32];

  for(Int_t c=0; c<N_CLASS; c++)
  {
    sprintf(pt_vs_eta_arr_n[c],"%s_pt_vs_eta_arr",class_name[c]);
    sprintf(en_vs_eta_arr_n[c],"%s_en_vs_eta_arr",class_name[c]);
    sprintf(pt_vs_phi_arr_n[c],"%s_pt_vs_phi_arr",class_name[c]);
    sprintf(en_vs_phi_arr_n[c],"%s_en_vs_phi_arr",class_name[c]);
    sprintf(eta_vs_phi_arr_n[c],"%s_eta_vs_phi_arr",class_name[c]);
    sprintf(pt_vs_en_arr_n[c],"%s_pt_vs_en_arr",class_name[c]);
    pt_vs_eta_arr[c] = new TObjArray();
    en_vs_eta_arr[c] = new TObjArray();
    pt_vs_phi_arr[c] = new TObjArray();
    en_vs_phi_arr[c] = new TObjArray();
    eta_vs_phi_arr[c] = new TObjArray();
    pt_vs_en_arr[c] = new TObjArray();
  };
  sprintf(pi0_z_vs_eta_arr_n,"%s_z_vs_eta_arr",class_name[kPi0]);
  sprintf(pi0_z_vs_phi_arr_n,"%s_z_vs_phi_arr",class_name[kPi0]);
  sprintf(pi0_mass_vs_en_arr_n,"%s_mass_vs_en_arr",class_name[kPi0]);
  sprintf(pi0_mass_vs_pt_arr_n,"%s_mass_vs_pt_arr",class_name[kPi0]);
  sprintf(pi0_mass_dist_arr_n,"%s_mass_dist_arr",class_name[kPi0]);
  sprintf(pi0_z_dist_arr_n,"%s_z_dist_arr",class_name[kPi0]);
  for(Int_t k=0; k<10; k++)
  {
    sprintf(mass_dist_for_enbin_arr_n[k],"mass_dist_for_enbin_%d_arr",k);
    sprintf(mass_dist_for_ptbin_arr_n[k],"mass_dist_for_ptbin_%d_arr",k);
    mass_dist_for_enbin_arr[k] = new TObjArray();
    mass_dist_for_ptbin_arr[k] = new TObjArray();
  };

  for(Int_t t=0; t<N_TRIG; t++)
  {
    for(Int_t c=0; c<N_CLASS; c++)
    {
      pt_vs_eta_arr[c]->AddLast(pt_vs_eta[c][t]);
      en_vs_eta_arr[c]->AddLast(en_vs_eta[c][t]);
      pt_vs_phi_arr[c]->AddLast(pt_vs_phi[c][t]);
      en_vs_phi_arr[c]->AddLast(en_vs_phi[c][t]);
      eta_vs_phi_arr[c]->AddLast(eta_vs_phi[c][t]);
      pt_vs_en_arr[c]->AddLast(pt_vs_en[c][t]);
    };
    pi0_z_vs_eta_arr->AddLast(pi0_z_vs_eta[t]);
    pi0_z_vs_phi_arr->AddLast(pi0_z_vs_phi[t]);
    pi0_mass_vs_en_arr->AddLast(pi0_mass_vs_en[t]);
    pi0_mass_vs_pt_arr->AddLast(pi0_mass_vs_pt[t]);
    pi0_mass_dist_arr->AddLast(pi0_mass_dist[t]);
    pi0_z_dist_arr->AddLast(pi0_z_dist[t]);
    for(Int_t k=0; k<10; k++)
    {
      mass_dist_for_enbin_arr[k]->AddLast(mass_dist_for_enbin[t][k]);
      mass_dist_for_ptbin_arr[k]->AddLast(mass_dist_for_ptbin[t][k]);
    };
  };


  // write output
  char outfilename[64];
  sprintf(outfilename,"diag.root");
  TFile * outfile = new TFile(outfilename,"RECREATE");

  trig_dist->Write();
  for(Int_t c=0; c<N_CLASS; c++)
  {
    pt_vs_eta_arr[c]->Write(pt_vs_eta_arr_n[c],TObject::kSingleKey);
    en_vs_eta_arr[c]->Write(en_vs_eta_arr_n[c],TObject::kSingleKey);
    pt_vs_phi_arr[c]->Write(pt_vs_phi_arr_n[c],TObject::kSingleKey);
    en_vs_phi_arr[c]->Write(en_vs_phi_arr_n[c],TObject::kSingleKey);
    eta_vs_phi_arr[c]->Write(eta_vs_phi_arr_n[c],TObject::kSingleKey);
    pt_vs_en_arr[c]->Write(pt_vs_en_arr_n[c],TObject::kSingleKey);
  };
  pi0_z_vs_eta_arr->Write(pi0_z_vs_eta_arr_n,TObject::kSingleKey);
  pi0_z_vs_phi_arr->Write(pi0_z_vs_phi_arr_n,TObject::kSingleKey);
  pi0_mass_vs_en_arr->Write(pi0_mass_vs_en_arr_n,TObject::kSingleKey);
  pi0_mass_vs_pt_arr->Write(pi0_mass_vs_pt_arr_n,TObject::kSingleKey);
  pi0_mass_dist_arr->Write(pi0_mass_dist_arr_n,TObject::kSingleKey);
  pi0_z_dist_arr->Write(pi0_z_dist_arr_n,TObject::kSingleKey);
  for(Int_t k=0; k<10; k++)
  {
    mass_dist_for_enbin_arr[k]->Write(mass_dist_for_enbin_arr_n[k],TObject::kSingleKey);
  };
  for(Int_t k=0; k<10; k++)
  {
    mass_dist_for_ptbin_arr[k]->Write(mass_dist_for_ptbin_arr_n[k],TObject::kSingleKey);
  };
};
