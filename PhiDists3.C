// -- create phi distributions with various kinematic cuts (eta,pt,E) for each spinbit
//    (kinematic cuts are set as Double_t's below)
// -- phi distributions for "+ -" and "- +" are weighted by relative luminosity
//    A_LL = 1/(P_b*P_y)*(N++ + N-- - RN+- -RN-+)/(N++ + N-- + RN+- + RN-+)
// -- phi distributions are written to phiset/ directory with similar name; 
//    they are named: phi_s[spinbit]_g[eta bin]_p[pt bin]_e[en bin]

void PhiDists3(const char * filename="RedOutputset070aa.root")
{
  // load polarization and rellum data
  gSystem->Load("src/RunData.so");
  RunData * RD = new RunData();
  Trigger * T = new Trigger();

  // get bins from environment
  Int_t phi_bins0, eta_bins0, pt_bins0, en_bins0;
  char trigger[16];
  char cut_type[16];
  if(gSystem->Getenv("TRIGGER")==NULL){fprintf(stderr,"ERROR: source env vars\n"); return;};
  sscanf(gSystem->Getenv("PHI_BINS"),"%d",&phi_bins0);
  sscanf(gSystem->Getenv("ETA_BINS"),"%d",&eta_bins0);
  sscanf(gSystem->Getenv("PT_BINS"),"%d",&pt_bins0);
  sscanf(gSystem->Getenv("EN_BINS"),"%d",&en_bins0);
  strcpy(trigger,gSystem->Getenv("TRIGGER"));
  strcpy(cut_type,gSystem->Getenv("CUT_TYPE"));
  const Int_t phi_bins = phi_bins0;
  const Int_t eta_bins = eta_bins0;
  const Int_t pt_bins = pt_bins0;
  const Int_t en_bins = en_bins0;
  Float_t phi_div[phi_bins+1];
  Float_t eta_div[eta_bins+1];
  Float_t pt_div[pt_bins+1];
  Float_t en_div[en_bins+1];
  char phi_div_env[phi_bins+1][20];
  char eta_div_env[eta_bins+1][20];
  char pt_div_env[pt_bins+1][20];
  char en_div_env[en_bins+1][20];
  for(Int_t i=0; i<=phi_bins; i++)
  {
    sprintf(phi_div_env[i],"PHI_DIV_%d",i);
    sscanf(gSystem->Getenv(phi_div_env[i]),"%f",&(phi_div[i]));
    printf("%s %f\n",phi_div_env[i],phi_div[i]);
  };
  for(Int_t i=0; i<=eta_bins; i++)
  {
    sprintf(eta_div_env[i],"ETA_DIV_%d",i);
    sscanf(gSystem->Getenv(eta_div_env[i]),"%f",&(eta_div[i]));
    printf("%s %f\n",eta_div_env[i],eta_div[i]);
  };
  for(Int_t i=0; i<=pt_bins; i++)
  {
    sprintf(pt_div_env[i],"PT_DIV_%d",i);
    sscanf(gSystem->Getenv(pt_div_env[i]),"%f",&(pt_div[i]));
    printf("%s %f\n",pt_div_env[i],pt_div[i]);
  };
  for(Int_t i=0; i<=en_bins; i++)
  {
    sprintf(en_div_env[i],"EN_DIV_%d",i);
    sscanf(gSystem->Getenv(en_div_env[i]),"%f",&(en_div[i]));
    printf("%s %f\n",en_div_env[i],en_div[i]);
  };
  Float_t phi_low; sscanf(gSystem->Getenv("PHI_LOW"),"%f",&phi_low);
  Float_t phi_high; sscanf(gSystem->Getenv("PHI_HIGH"),"%f",&phi_high);
  Float_t eta_low; sscanf(gSystem->Getenv("ETA_LOW"),"%f",&eta_low);
  Float_t eta_high; sscanf(gSystem->Getenv("ETA_HIGH"),"%f",&eta_high);
  Float_t pt_low; sscanf(gSystem->Getenv("PT_LOW"),"%f",&pt_low);
  Float_t pt_high; sscanf(gSystem->Getenv("PT_HIGH"),"%f",&pt_high);
  Float_t en_low; sscanf(gSystem->Getenv("EN_LOW"),"%f",&en_low);
  Float_t en_high; sscanf(gSystem->Getenv("EN_HIGH"),"%f",&en_high);


  // read redset tree and set output file name
  Int_t runnum,bx,blue,yell,pattern;
  Float_t M12,N12,E12,Z,Phi,Eta,Pt,b_pol,y_pol;
  Bool_t kicked,isConsistent;
  Int_t L2sum[2];
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


  // define spinbit strings
  char spinbit_t[4][4];
  char spinbit_n[4][4];
  strcpy(spinbit_t[0],"--"); strcpy(spinbit_n[0],"nn");
  strcpy(spinbit_t[1],"-+"); strcpy(spinbit_n[1],"np");
  strcpy(spinbit_t[2],"+-"); strcpy(spinbit_n[2],"pn");
  strcpy(spinbit_t[3],"++"); strcpy(spinbit_n[3],"pp");


  // get number of runs in the redset file and build array of run numbers
  Int_t runnum_arr[30]; // (n.b. maximum size arbitrarily defined)
  Int_t runnum_tmp=0;
  Int_t NRUNS_tmp = 0;
  if(tree->GetEntries() == 0)
  {
    fprintf(stderr,"ERROR: no str entries for %s\n --> phi file not produced!\n",filename);
    return;
  };
  for(Int_t i=0; i<tree->GetEntries(); i++)
  {
    tree->GetEntry(i);
    if(runnum != runnum_tmp)
    {
      runnum_tmp = runnum;
      if(NRUNS_tmp<30) runnum_arr[NRUNS_tmp] = runnum;
      else 
      {
        fprintf(stderr,"ERROR: more than 30 runs in root file; increase arbitrarily defined max\n");
        return;
      };
      NRUNS_tmp++;
    };
  };
  const Int_t NRUNS = NRUNS_tmp;
  for(Int_t r=0; r<NRUNS; r++) printf("%d\n",runnum_arr[r]);
  printf("NRUNS=%d\n",NRUNS);


  // read mass cuts (produced using MassCutter.C)
  // -- used to implement (energy or pt)-dependent mass cuts for pi0s
  //    since the mass peak drifts w.r.t. energy bin
  // -- NOTE: THIS MAY SLIGHTLY CHANGE THE ENERGY BIN BOUNDARIES FROM WHAT'S IN ENV!!!!!!!!!
  TTree * mass_cut_tr = new TTree("mass_cut_tr","mass_cut_tr");
  mass_cut_tr->ReadFile("mass_cuts.dat","lb/F:ub/F:massL/F:massM/F:massH/F");
  Int_t mass_cut_tr_ent_tmp = mass_cut_tr->GetEntries();
  const Int_t MENT = mass_cut_tr_ent_tmp;
  Float_t lb,ub,massL,massH;
  mass_cut_tr->SetBranchAddress("lb",&lb);
  mass_cut_tr->SetBranchAddress("ub",&ub);
  mass_cut_tr->SetBranchAddress("massL",&massL);
  mass_cut_tr->SetBranchAddress("massH",&massH);



  // define kinematic distributions ("wdist" = weighting distribution)
  // -- these are distributions in energy and phi, but with finer binning; they are used 
  //    to weight the horizontal position of points on the asymmetry plots
  // -- one pt wdist for each en bin (and eta bin) 
  // -- one en wdist for each pt bin (and eta bin)
  // -- one invariant mass (mm) wdist for each eta,pt,en bin
  const Int_t NWBINS = 100;
  TH1D * pt_wdist[3][eta_bins][en_bins][NRUNS]; // [jet type] [....]
  TH1D * en_wdist[3][eta_bins][pt_bins][NRUNS];
  TH1D * mm_wdist[3][eta_bins][pt_bins][en_bins][NRUNS];
  char pt_wdist_n[3][eta_bins][en_bins][NRUNS][64];
  char en_wdist_n[3][eta_bins][pt_bins][NRUNS][64];
  char mm_wdist_n[3][eta_bins][pt_bins][en_bins][NRUNS][64];
  for(Int_t r=0; r<NRUNS; r++)
  {
    for(Int_t g=0; g<eta_bins; g++)
    {
      for(Int_t e=0; e<en_bins; e++)
      {
        sprintf(pt_wdist_n[0][g][e][r],"pt_wdist_sph_g%d_e%d_r%d",g,e,runnum_arr[r]);
        sprintf(pt_wdist_n[1][g][e][r],"pt_wdist_pi0_g%d_e%d_r%d",g,e,runnum_arr[r]);
        sprintf(pt_wdist_n[2][g][e][r],"pt_wdist_thr_g%d_e%d_r%d",g,e,runnum_arr[r]);
        for(Int_t j=0; j<3; j++)
          pt_wdist[j][g][e][r] = new TH1D(pt_wdist_n[j][g][e][r],pt_wdist_n[j][g][e][r],NWBINS,pt_low,pt_high);
      };
      for(Int_t p=0; p<pt_bins; p++)
      {
        sprintf(en_wdist_n[0][g][p][r],"en_wdist_sph_g%d_p%d_r%d",g,p,runnum_arr[r]);
        sprintf(en_wdist_n[1][g][p][r],"en_wdist_pi0_g%d_p%d_r%d",g,p,runnum_arr[r]);
        sprintf(en_wdist_n[2][g][p][r],"en_wdist_thr_g%d_p%d_r%d",g,p,runnum_arr[r]);
        for(Int_t j=0; j<3; j++)
          en_wdist[j][g][p][r] = new TH1D(en_wdist_n[j][g][p][r],en_wdist_n[j][g][p][r],NWBINS,en_low,en_high);
      };
      for(Int_t p=0; p<pt_bins; p++)
      {
        for(Int_t e=0; e<en_bins; e++)
        {
          sprintf(mm_wdist_n[0][g][p][e][r],"mm_wdist_sph_g%d_p%d_e%d_r%d",g,p,e,runnum_arr[r]);
          sprintf(mm_wdist_n[1][g][p][e][r],"mm_wdist_pi0_g%d_p%d_e%d_r%d",g,p,e,runnum_arr[r]);
          sprintf(mm_wdist_n[2][g][p][e][r],"mm_wdist_thr_g%d_p%d_e%d_r%d",g,p,e,runnum_arr[r]);
          for(Int_t j=0; j<3; j++)
            mm_wdist[j][g][p][e][r] = new TH1D(mm_wdist_n[j][g][p][e][r],mm_wdist_n[j][g][p][e][r],NWBINS,0,1);
        };
      };
    };
  };


  // define phi distributions  for each spinbit and kinematic bin
  // _sph = single photon
  // _pi0 = neutral pions
  // _thr = >=3 photon jet
  // ---> n.b. I can't use higher dimensional arrays
  TH1D * phi_dist_sph[4][eta_bins][pt_bins][en_bins][NRUNS]; // [spinbit] [eta] [pt] [en] [run number]
  TH1D * phi_dist_pi0[4][eta_bins][pt_bins][en_bins][NRUNS]; 
  TH1D * phi_dist_thr[4][eta_bins][pt_bins][en_bins][NRUNS]; 

  char phi_dist_sph_n[4][eta_bins][pt_bins][en_bins][NRUNS][256];
  char phi_dist_pi0_n[4][eta_bins][pt_bins][en_bins][NRUNS][256];
  char phi_dist_thr_n[4][eta_bins][pt_bins][en_bins][NRUNS][256];

  char phi_dist_sph_t[4][eta_bins][pt_bins][en_bins][NRUNS][400];
  char phi_dist_pi0_t[4][eta_bins][pt_bins][en_bins][NRUNS][400];
  char phi_dist_thr_t[4][eta_bins][pt_bins][en_bins][NRUNS][400];

  for(Int_t r=0; r<NRUNS; r++)
  {
    for(Int_t s=0; s<4; s++)
    {
      for(Int_t g=0; g<eta_bins; g++)
      {
        for(Int_t p=0; p<pt_bins; p++)
        {
          for(Int_t e=0; e<en_bins; e++)
          {
            sprintf(phi_dist_sph_n[s][g][p][e][r],"phi_sph_s%d_g%d_p%d_e%d_r%d",s,g,p,e,runnum_arr[r]);
            sprintf(phi_dist_pi0_n[s][g][p][e][r],"phi_pi0_s%d_g%d_p%d_e%d_r%d",s,g,p,e,runnum_arr[r]);
            sprintf(phi_dist_thr_n[s][g][p][e][r],"phi_thr_s%d_g%d_p%d_e%d_r%d",s,g,p,e,runnum_arr[r]);

            sprintf(phi_dist_sph_t[s][g][p][e][r],
             "single-#gamma #phi distribution :: spin=(%s) #eta#in[%.2f,%.2f) p_{T}#in[%.2f,%.2f) E#in[%.2f,%.2f) :: r%d",
             spinbit_t[s],eta_div[g],eta_div[g+1],pt_div[p],pt_div[p+1],en_div[e],en_div[e+1],runnum_arr[r]);
            sprintf(phi_dist_pi0_t[s][g][p][e][r],
             "#pi^{0} #phi distribution :: spin=(%s) #eta#in[%.2f,%.2f) p_{T}#in[%.2f,%.2f) E#in[%.2f,%.2f) :: r%d",
             spinbit_t[s],eta_div[g],eta_div[g+1],pt_div[p],pt_div[p+1],en_div[e],en_div[e+1],runnum_arr[r]);
            sprintf(phi_dist_thr_t[s][g][p][e][r],
             "N_{#gamma}>3 jet #phi distribution :: spin=(%s) #eta#in[%.2f,%.2f) p_{T}#in[%.2f,%.2f) E#in[%.2f,%.2f) :: r%d",
             spinbit_t[s],eta_div[g],eta_div[g+1],pt_div[p],pt_div[p+1],en_div[e],en_div[e+1],runnum_arr[r]);

            phi_dist_sph[s][g][p][e][r] = new TH1D(phi_dist_sph_n[s][g][p][e][r],phi_dist_sph_t[s][g][p][e][r],
              phi_bins,phi_low,phi_high);
            phi_dist_pi0[s][g][p][e][r] = new TH1D(phi_dist_pi0_n[s][g][p][e][r],phi_dist_pi0_t[s][g][p][e][r],
              phi_bins,phi_low,phi_high);
            phi_dist_thr[s][g][p][e][r] = new TH1D(phi_dist_thr_n[s][g][p][e][r],phi_dist_thr_t[s][g][p][e][r],
              phi_bins,phi_low,phi_high);

            phi_dist_sph[s][g][p][e][r]->Sumw2();
            phi_dist_pi0[s][g][p][e][r]->Sumw2();
            phi_dist_thr[s][g][p][e][r]->Sumw2();
          };
        }; 
      };
    };
  };

  
  // load run exclusion lists (for excluding specific runs, e.g., due to hot tower)
  // -- exclusion_list_from_situ is the full list runs with pathological pt distributions
  //    along with explanations of their pathologies
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


  // fill phi distributions and wdists 
  Int_t ss,gg,pp,ee,rr;
  rr=-1; runnum_tmp=0;
  printf("fill phi dists...\n");
  Bool_t usepi0;
  for(Int_t x=0; x<tree->GetEntries(); x++)
  {
    if((x%10000)==0) printf("%.2f%%\n",100*((Float_t)x)/((Float_t)tree->GetEntries()));
    ss=gg=pp=ee=-1; // reset 
    tree->GetEntry(x);
    blue = RD->BlueSpin(runnum,bx);
    yell = RD->YellSpin(runnum,bx);
    kicked = RD->Kicked(runnum,bx);

    // spin --> array index
    if(blue==-1 && yell==-1) ss=0;
    else if(blue==-1 && yell==1) ss=1;
    else if(blue==1 && yell==-1) ss=2;
    else if(blue==1 && yell==1) ss=3;

    // kinematic bins --> array indices
    for(Int_t g=0; g<eta_bins; g++) { if(Eta>=eta_div[g] && Eta<eta_div[g+1]) gg=g; };
    for(Int_t p=0; p<pt_bins;  p++) { if(Pt>=pt_div[p]   && Pt<pt_div[p+1]  ) pp=p; };
    for(Int_t e=0; e<en_bins;  e++) { if(E12>=en_div[e]  && E12<en_div[e+1] ) ee=e; };

    // run number --> array index
    if(runnum != runnum_tmp)
    {
      rr=-1;
      for(Int_t r=0; r<NRUNS; r++) { if(runnum_arr[r] == runnum) rr=r; }; 
      runnum_tmp = runnum;
      isConsistent = RD->RellumConsistent(runnum);
      pattern = RD->Pattern(runnum);
      b_pol = RD->BluePol(runnum);
      y_pol = RD->YellPol(runnum);
      exclude_sph=0;
      exclude_pi0=0;
      exclude_thr=0;
      for(Int_t xx=0; xx<exclusion_sph->GetEntries(); xx++) { exclusion_sph->GetEntry(xx); if(runnum==rn_sph) exclude_sph=1; };
      for(Int_t xx=0; xx<exclusion_pi0->GetEntries(); xx++) { exclusion_pi0->GetEntry(xx); if(runnum==rn_pi0) exclude_pi0=1; };
      for(Int_t xx=0; xx<exclusion_thr->GetEntries(); xx++) { exclusion_thr->GetEntry(xx); if(runnum==rn_thr) exclude_thr=1; };
    };

    // check for valid array indices (filters out events outside kinematic boundaries)
    //printf("%d %d %d %d %d\n",ss,gg,pp,ee,rr);
    if(ss>=0 && gg>=0 && pp>=0 && ee>=0 && rr>=0)
    {
      // rellum consistency, polarization, and trigger cut
      if( kicked==0 && isConsistent==1 && b_pol>0 && y_pol>0 && (L2sum[1]&T->Mask(runnum,trigger,1)))
      {
        // single photon cut
        if( exclude_sph==0 && fabs(N12-1)<0.01 )
        {
          phi_dist_sph[ss][gg][pp][ee][rr]->Fill(Phi);
          pt_wdist[0][gg][ee][rr]->Fill(Pt);
          en_wdist[0][gg][pp][rr]->Fill(E12);
          mm_wdist[0][gg][pp][ee][rr]->Fill(M12);
        }

        // pi0 cut (includes hard kinematic cutoffs overriding env variables)
        else if(  exclude_pi0==0 &&
                  fabs(N12-2)<0.01 &&
                  Z<0.8 &&
                  Pt>=0 && Pt<10 &&
                  E12>=0 && E12<100
                  )
        {
          // fill mass wdist regardless if event passes mass cut; this is to look at full
          // mass distributions for each run, which is useful for hunting for hot towers
          mm_wdist[1][gg][pp][ee][rr]->Fill(M12); 

          // $CUT_TYPE-dependent mass cut
          usepi0=false;
          for(Int_t qq=0; qq<mass_cut_tr->GetEntries(); qq++)
          {
            mass_cut_tr->GetEntry(qq);
            if(((!strcmp(cut_type,"en") && E12>=lb && E12<ub) ||
                (!strcmp(cut_type,"pt") && Pt>=lb && Pt<lb)) &&
                M12>=massL && M12<massH) usepi0=true;
          };
          if(usepi0)
          {
            phi_dist_pi0[ss][gg][pp][ee][rr]->Fill(Phi);
            pt_wdist[1][gg][ee][rr]->Fill(Pt);
            en_wdist[1][gg][pp][rr]->Fill(E12);
          };
        }


        // >=3 photon jet cut
        else if( exclude_thr==0 && N12>2 )
        {
          phi_dist_thr[ss][gg][pp][ee][rr]->Fill(Phi);
          pt_wdist[2][gg][ee][rr]->Fill(Pt);
          en_wdist[2][gg][pp][rr]->Fill(E12);
          mm_wdist[2][gg][pp][ee][rr]->Fill(M12);
        };
      };
    };
  };
        

  // make object arrays
  TFile * outfile = new TFile(outname,"RECREATE");
  TObjArray * phi_dist_sph_arr[4][eta_bins][pt_bins][en_bins];
  TObjArray * phi_dist_pi0_arr[4][eta_bins][pt_bins][en_bins];
  TObjArray * phi_dist_thr_arr[4][eta_bins][pt_bins][en_bins];
  TObjArray * pt_wdist_arr[3][eta_bins][en_bins];
  TObjArray * en_wdist_arr[3][eta_bins][pt_bins];
  TObjArray * mm_wdist_arr[3][eta_bins][pt_bins][en_bins];
  char phi_dist_sph_arr_name[4][eta_bins][pt_bins][en_bins][128];
  char phi_dist_pi0_arr_name[4][eta_bins][pt_bins][en_bins][128];
  char phi_dist_thr_arr_name[4][eta_bins][pt_bins][en_bins][128];
  char pt_wdist_arr_name[3][eta_bins][en_bins][128];
  char en_wdist_arr_name[3][eta_bins][pt_bins][128];
  char mm_wdist_arr_name[3][eta_bins][pt_bins][en_bins][128];
  for(Int_t e=0; e<en_bins; e++)
  {
    for(Int_t g=0; g<eta_bins; g++)
    {
      for(Int_t p=0; p<pt_bins; p++)
      {
        for(Int_t s=0; s<4; s++)
        {
          phi_dist_sph_arr[s][g][p][e] = new TObjArray();
          phi_dist_pi0_arr[s][g][p][e] = new TObjArray();
          phi_dist_thr_arr[s][g][p][e] = new TObjArray();

          sprintf(phi_dist_sph_arr_name[s][g][p][e],"phi_dist_sph_s%d_g%d_p%d_e%d",s,g,p,e);
          sprintf(phi_dist_pi0_arr_name[s][g][p][e],"phi_dist_pi0_s%d_g%d_p%d_e%d",s,g,p,e);
          sprintf(phi_dist_thr_arr_name[s][g][p][e],"phi_dist_thr_s%d_g%d_p%d_e%d",s,g,p,e);
          
          for(Int_t r=0; r<NRUNS; r++)
          {
            phi_dist_sph_arr[s][g][p][e]->AddLast(phi_dist_sph[s][g][p][e][r]);
            phi_dist_pi0_arr[s][g][p][e]->AddLast(phi_dist_pi0[s][g][p][e][r]);
            phi_dist_thr_arr[s][g][p][e]->AddLast(phi_dist_thr[s][g][p][e][r]);
          };
        };
      };
    };
  };
  for(Int_t g=0; g<eta_bins; g++)
  {
    for(Int_t e=0; e<en_bins; e++)
    {
      for(Int_t j=0; j<3; j++) pt_wdist_arr[j][g][e] = new TObjArray();
      sprintf(pt_wdist_arr_name[0][g][e],"pt_wdist_sph_g%d_e%d",g,e);
      sprintf(pt_wdist_arr_name[1][g][e],"pt_wdist_pi0_g%d_e%d",g,e);
      sprintf(pt_wdist_arr_name[2][g][e],"pt_wdist_thr_g%d_e%d",g,e);
      for(Int_t j=0; j<3; j++) for(Int_t r=0; r<NRUNS; r++) pt_wdist_arr[j][g][e]->AddLast(pt_wdist[j][g][e][r]);
    };
    for(Int_t p=0; p<pt_bins; p++)
    {
      for(Int_t j=0; j<3; j++) en_wdist_arr[j][g][p] = new TObjArray();
      sprintf(en_wdist_arr_name[0][g][p],"en_wdist_sph_g%d_p%d",g,p);
      sprintf(en_wdist_arr_name[1][g][p],"en_wdist_pi0_g%d_p%d",g,p);
      sprintf(en_wdist_arr_name[2][g][p],"en_wdist_thr_g%d_p%d",g,p);
      for(Int_t j=0; j<3; j++) for(Int_t r=0; r<NRUNS; r++) en_wdist_arr[j][g][p]->AddLast(en_wdist[j][g][p][r]);
    };
    for(Int_t p=0; p<pt_bins; p++)
    {
      for(Int_t e=0; e<en_bins; e++)
      {
        for(Int_t j=0; j<3; j++) mm_wdist_arr[j][g][p][e] = new TObjArray();
        sprintf(mm_wdist_arr_name[0][g][p][e],"mm_wdist_sph_g%d_p%d_e%d",g,p,e);
        sprintf(mm_wdist_arr_name[1][g][p][e],"mm_wdist_pi0_g%d_p%d_e%d",g,p,e);
        sprintf(mm_wdist_arr_name[2][g][p][e],"mm_wdist_thr_g%d_p%d_e%d",g,p,e);
        for(Int_t j=0; j<3; j++) for(Int_t r=0; r<NRUNS; r++) mm_wdist_arr[j][g][p][e]->AddLast(mm_wdist[j][g][p][e][r]);
      };
    };
  };

  
  // write object arrays
  outfile->mkdir("sph");
  outfile->cd("sph");
  for(Int_t e=0; e<en_bins; e++)
  {
    for(Int_t g=0; g<eta_bins; g++)
    {
      for(Int_t p=0; p<pt_bins; p++)
      {
        for(Int_t s=0; s<4; s++)
        {
          phi_dist_sph_arr[s][g][p][e]->Write(phi_dist_sph_arr_name[s][g][p][e],TObject::kSingleKey);
        };
      };
    };
  };
  outfile->cd();
  outfile->mkdir("pi0");
  outfile->cd("pi0");
  for(Int_t e=0; e<en_bins; e++)
  {
    for(Int_t g=0; g<eta_bins; g++)
    {
      for(Int_t p=0; p<pt_bins; p++)
      {
        for(Int_t s=0; s<4; s++)
        {
          phi_dist_pi0_arr[s][g][p][e]->Write(phi_dist_pi0_arr_name[s][g][p][e],TObject::kSingleKey);
        };
      };
    };
  };
  outfile->cd();
  outfile->mkdir("thr");
  outfile->cd("thr");
  for(Int_t e=0; e<en_bins; e++)
  {
    for(Int_t g=0; g<eta_bins; g++)
    {
      for(Int_t p=0; p<pt_bins; p++)
      {
        for(Int_t s=0; s<4; s++)
        {
          phi_dist_thr_arr[s][g][p][e]->Write(phi_dist_thr_arr_name[s][g][p][e],TObject::kSingleKey);
        };
      };
    };
  };
  outfile->cd();

  // write wdists
  for(Int_t g=0; g<eta_bins; g++)
  {
    for(Int_t j=0; j<3; j++)
    {
      for(Int_t e=0; e<en_bins; e++) pt_wdist_arr[j][g][e]->Write(pt_wdist_arr_name[j][g][e],TObject::kSingleKey);
      for(Int_t p=0; p<pt_bins; p++) en_wdist_arr[j][g][p]->Write(en_wdist_arr_name[j][g][p],TObject::kSingleKey);
      for(Int_t p=0; p<pt_bins; p++) for(Int_t e=0; e<en_bins; e++)
        mm_wdist_arr[j][g][p][e]->Write(mm_wdist_arr_name[j][g][p][e],TObject::kSingleKey);
    };
  };
};
