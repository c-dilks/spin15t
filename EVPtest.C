// test EVP calculation
//

void EVPtest(const char * infile_name = "RedOutputset079ai.root")
{
  gStyle->SetOptStat(0);
  // if enableOverlap=true, fill overlap matrices; I've moved this method to 
  // TriggerBooleanOverlap.C in hopes of making this script faster.. it's use
  // here is deprecated
  const Bool_t enableOverlap = false;

  const Int_t NBINS=5; // NUMBER OF BINS (default 400)
  const Int_t NBINS_RDIST=5; // number of bins for variable vs. run index plots (default 100)
  const Int_t MAXRUNS=12; // arbitrary max number of runs in redset file 

  enum ew_enum {kE,kW};
  enum sl_enum {kS,kL};

  TString ew_str[2];
  TString sl_str[2];
  ew_str[kE]="E";
  ew_str[kW]="W";
  sl_str[kS]="small";
  sl_str[kL]="large";

  gSystem->Load("src/RunInfo.so");
  RunInfo * RD = new RunInfo();
  LevelTwo * T = new LevelTwo();
  Environ * env = new Environ();
  EventClass * ev = new EventClass();
  TriggerBoolean * trg_bool = 
    new TriggerBoolean(env->STG1,env->STG2,env->MIPN,env->USE_TCU_BITS);
  BBCtiles * bbc = new BBCtiles();


  // open tree
  char infile_full_name[256];
  sprintf(infile_full_name,"redset/%s",infile_name);
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

  int ew,sl,chan;

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

  /*
  tr->SetBranchAddress("RPE_QTN",&(trg_bool->RPSCI->N[kE]));
  tr->SetBranchAddress("RPW_QTN",&(trg_bool->RPSCI->N[kW]));
  tr->SetBranchAddress("RPE_Idx",trg_bool->RPSCI->Idx[kE]);
  tr->SetBranchAddress("RPE_TAC",trg_bool->RPSCI->TAC[kE]);
  tr->SetBranchAddress("RPE_ADC",trg_bool->RPSCI->ADC[kE]);
  tr->SetBranchAddress("RPW_Idx",trg_bool->RPSCI->Idx[kW]);
  tr->SetBranchAddress("RPW_TAC",trg_bool->RPSCI->TAC[kW]);
  tr->SetBranchAddress("RPW_ADC",trg_bool->RPSCI->ADC[kW]);
  tr->SetBranchAddress("RPvertex",trg_bool->RPSCI->vertex);
  */

  tr->SetBranchAddress("BBCE_QTN",&(bbc->QTN[kE][0]));
  tr->SetBranchAddress("BBCW_QTN",&(bbc->QTN[kW][0]));
  tr->SetBranchAddress("BBCE_Idx",bbc->Idx[kE][0]);
  tr->SetBranchAddress("BBCE_ADC",bbc->ADC[kE][0]);
  tr->SetBranchAddress("BBCE_TAC",bbc->TAC[kE][0]);
  tr->SetBranchAddress("BBCW_Idx",bbc->Idx[kW][0]);
  tr->SetBranchAddress("BBCW_ADC",bbc->ADC[kW][0]);
  tr->SetBranchAddress("BBCW_TAC",bbc->TAC[kW][0]);
  tr->SetBranchAddress("BBCvertex",&(bbc->vertex));

  // BBC histograms
  TH1D * adc[2][2][16]; // [ew] [sl] [chan = pmt-1]
  TH1D * tac[2][2][16]; // [ew] [sl] [chan = pmt-1]
  TH2D * adc_vs_tac[2][2][16]; 
  TString adc_n[2][2][16];
  TString tac_n[2][2][16];
  TString adc_vs_tac_n[2][2][16];

  for(ew=0; ew<2; ew++)
  {
    for(sl=0; sl<2; sl++)
    {
      for(chan=0; chan<16; chan++)
      {
        adc_n[ew][sl][chan] = Form("adc_%s_%s_pmt_%d",ew_str[ew].Data(),sl_str[sl].Data(),chan+1);
        tac_n[ew][sl][chan] = Form("tac_%s_%s_pmt_%d",ew_str[ew].Data(),sl_str[sl].Data(),chan+1);
        adc_vs_tac_n[ew][sl][chan] = Form("adc_vs_tac_%s_%s_pmt_%d",ew_str[ew].Data(),sl_str[sl].Data(),chan+1);
        adc[ew][sl][chan] = new TH1D(adc_n[ew][sl][chan].Data(),adc_n[ew][sl][chan].Data(),1000,0,4096);
        tac[ew][sl][chan] = new TH1D(tac_n[ew][sl][chan].Data(),tac_n[ew][sl][chan].Data(),1000,0,4096);
        adc_vs_tac[ew][sl][chan] = new TH2D(adc_vs_tac_n[ew][sl][chan].Data(),adc_vs_tac_n[ew][sl][chan].Data(),
            200,0,4096,200,0,4096);
      };
    };
  };



  // define output tree
  char outfilename[256];
  char RP_suffix[16];
  sscanf(infile_name,"RedOutput%s",outfilename);
  if(!strcmp(env->RPselect,"")) strcpy(RP_suffix,"");
  else sprintf(RP_suffix,"_%s",env->RPselect);
  sprintf(outfilename,"evpset/diag%s.%s",RP_suffix,outfilename);
  TFile * outfile = new TFile(outfilename,"RECREATE");

  TTree * outtr = new TTree();
  Int_t ou_qtn[2];
  Double_t ou_evp[2][2];
  Double_t ou_evp_cor[2][2];
  Double_t esum[2][2];
  Double_t Xflow[2][2];
  Double_t Yflow[2][2];
  Double_t Xflow_cor[2][2];
  Double_t Yflow_cor[2][2];
  Double_t sigma_x[2][2];
  Double_t sigma_y[2][2];
  Double_t sigma_xy[2][2];
  Double_t sigma_min[2][2];
  Double_t sigma_max[2][2];
  Double_t sigma_theta[2][2];
  Bool_t ou_H,ou_V;
  outtr->Branch("qtne",&(ou_qtn[kE]),"qtne/I");
  outtr->Branch("qtnw",&(ou_qtn[kW]),"qtnw/I");

  outtr->Branch("evp_es",&(ou_evp[kE][0]),"evp_es/D");
  outtr->Branch("evp_ws",&(ou_evp[kW][0]),"evp_ws/D");
  outtr->Branch("evp_el",&(ou_evp[kE][1]),"evp_el/D");
  outtr->Branch("evp_wl",&(ou_evp[kW][1]),"evp_wl/D");

  outtr->Branch("evp_cor_es",&(ou_evp_cor[kE][0]),"evp_cor_es/D");
  outtr->Branch("evp_cor_ws",&(ou_evp_cor[kW][0]),"evp_cor_ws/D");
  outtr->Branch("evp_cor_el",&(ou_evp_cor[kE][1]),"evp_cor_el/D");
  outtr->Branch("evp_cor_wl",&(ou_evp_cor[kW][1]),"evp_cor_wl/D");

  outtr->Branch("esum_es",&(esum[kE][0]),"esum_es/D");
  outtr->Branch("esum_ws",&(esum[kW][0]),"esum_ws/D");
  outtr->Branch("esum_el",&(esum[kE][1]),"esum_el/D");
  outtr->Branch("esum_wl",&(esum[kW][1]),"esum_wl/D");

  outtr->Branch("Xflow_es",&(Xflow[kE][0]),"Xflow_es/D");
  outtr->Branch("Xflow_ws",&(Xflow[kW][0]),"Xflow_ws/D");
  outtr->Branch("Xflow_el",&(Xflow[kE][1]),"Xflow_el/D");
  outtr->Branch("Xflow_wl",&(Xflow[kW][1]),"Xflow_wl/D");

  outtr->Branch("Yflow_es",&(Yflow[kE][0]),"Yflow_es/D");
  outtr->Branch("Yflow_ws",&(Yflow[kW][0]),"Yflow_ws/D");
  outtr->Branch("Yflow_el",&(Yflow[kE][1]),"Yflow_el/D");
  outtr->Branch("Yflow_wl",&(Yflow[kW][1]),"Yflow_wl/D");

  outtr->Branch("Xflow_cor_es",&(Xflow_cor[kE][0]),"Xflow_cor_es/D");
  outtr->Branch("Xflow_cor_ws",&(Xflow_cor[kW][0]),"Xflow_cor_ws/D");
  outtr->Branch("Xflow_cor_el",&(Xflow_cor[kE][1]),"Xflow_cor_el/D");
  outtr->Branch("Xflow_cor_wl",&(Xflow_cor[kW][1]),"Xflow_cor_wl/D");

  outtr->Branch("Yflow_cor_es",&(Yflow_cor[kE][0]),"Yflow_cor_es/D");
  outtr->Branch("Yflow_cor_ws",&(Yflow_cor[kW][0]),"Yflow_cor_ws/D");
  outtr->Branch("Yflow_cor_el",&(Yflow_cor[kE][1]),"Yflow_cor_el/D");
  outtr->Branch("Yflow_cor_wl",&(Yflow_cor[kW][1]),"Yflow_cor_wl/D");

  outtr->Branch("sigma_x_es",&(sigma_x[kE][0]),"sigma_x_es/D");
  outtr->Branch("sigma_x_ws",&(sigma_x[kW][0]),"sigma_x_ws/D");
  outtr->Branch("sigma_x_el",&(sigma_x[kE][1]),"sigma_x_el/D");
  outtr->Branch("sigma_x_wl",&(sigma_x[kW][1]),"sigma_x_wl/D");

  outtr->Branch("sigma_y_es",&(sigma_y[kE][0]),"sigma_y_es/D");
  outtr->Branch("sigma_y_ws",&(sigma_y[kW][0]),"sigma_y_ws/D");
  outtr->Branch("sigma_y_el",&(sigma_y[kE][1]),"sigma_y_el/D");
  outtr->Branch("sigma_y_wl",&(sigma_y[kW][1]),"sigma_y_wl/D");

  outtr->Branch("sigma_xy_es",&(sigma_xy[kE][0]),"sigma_xy_es/D");
  outtr->Branch("sigma_xy_ws",&(sigma_xy[kW][0]),"sigma_xy_ws/D");
  outtr->Branch("sigma_xy_el",&(sigma_xy[kE][1]),"sigma_xy_el/D");
  outtr->Branch("sigma_xy_wl",&(sigma_xy[kW][1]),"sigma_xy_wl/D");

  outtr->Branch("sigma_min_es",&(sigma_min[kE][0]),"sigma_min_es/D");
  outtr->Branch("sigma_min_ws",&(sigma_min[kW][0]),"sigma_min_ws/D");
  outtr->Branch("sigma_min_el",&(sigma_min[kE][1]),"sigma_min_el/D");
  outtr->Branch("sigma_min_wl",&(sigma_min[kW][1]),"sigma_min_wl/D");

  outtr->Branch("sigma_max_es",&(sigma_max[kE][0]),"sigma_max_es/D");
  outtr->Branch("sigma_max_ws",&(sigma_max[kW][0]),"sigma_max_ws/D");
  outtr->Branch("sigma_max_el",&(sigma_max[kE][1]),"sigma_max_el/D");
  outtr->Branch("sigma_max_wl",&(sigma_max[kW][1]),"sigma_max_wl/D");

  outtr->Branch("sigma_theta_es",&(sigma_theta[kE][0]),"sigma_theta_es/D");
  outtr->Branch("sigma_theta_ws",&(sigma_theta[kW][0]),"sigma_theta_ws/D");
  outtr->Branch("sigma_theta_el",&(sigma_theta[kE][1]),"sigma_theta_el/D");
  outtr->Branch("sigma_theta_wl",&(sigma_theta[kW][1]),"sigma_theta_wl/D");

  outtr->Branch("vertex",&(bbc->vertex),"vertex/F");

  outtr->Branch("V",&(ou_V),"V/O");
  outtr->Branch("H",&(ou_H),"H/O");


  // trigger list
  Int_t N_TRIG_tmp = T->N;
  const Int_t N_TRIG = N_TRIG_tmp;

  // event classes
  Int_t N_CLASS_tmp = ev->N;
  const Int_t N_CLASS = N_CLASS_tmp;


  // get kinematic ranges using the current binning set by Bin_Splitter.C
  Phi_min = env->PhiLow;
  Phi_max = env->PhiHigh;
  Eta_min = env->EtaLow;
  Eta_max = env->EtaHigh;
  E12_min = env->EnLow;
  E12_max = env->EnHigh;
  Pt_min = env->PtLow;
  Pt_max = env->PtHigh;

  
  // define histograms
  TH2D * pt_vs_eta[N_CLASS][N_TRIG];
  TH2D * en_vs_eta[N_CLASS][N_TRIG];
  TH2D * pt_vs_phi[N_CLASS][N_TRIG];
  TH2D * en_vs_phi[N_CLASS][N_TRIG];
  TH2D * eta_vs_phi[N_CLASS][N_TRIG];
  TH2D * pt_vs_en[N_CLASS][N_TRIG];
  TH2D * y_vs_x[N_CLASS][N_TRIG];

  TH2D * z_vs_eta[N_CLASS][N_TRIG];
  TH2D * z_vs_phi[N_CLASS][N_TRIG];
  TH2D * mass_vs_en[N_CLASS][N_TRIG];
  TH2D * mass_vs_pt[N_CLASS][N_TRIG];
  TH1D * mass_dist[N_CLASS][N_TRIG];
  TH1D * z_dist[N_CLASS][N_TRIG];

  TH1D * pt_rdist[N_CLASS][N_TRIG][MAXRUNS];
  TH1D * en_rdist[N_CLASS][N_TRIG][MAXRUNS];
  TH1D * eta_rdist[N_CLASS][N_TRIG][MAXRUNS];
  TH1D * phi_rdist[N_CLASS][N_TRIG][MAXRUNS];
  TH1D * mass_rdist[N_CLASS][N_TRIG][MAXRUNS];
  TH1D * z_rdist[N_CLASS][N_TRIG][MAXRUNS];

  char pt_vs_eta_n[N_CLASS][N_TRIG][64];
  char en_vs_eta_n[N_CLASS][N_TRIG][64];
  char pt_vs_phi_n[N_CLASS][N_TRIG][64];
  char en_vs_phi_n[N_CLASS][N_TRIG][64];
  char eta_vs_phi_n[N_CLASS][N_TRIG][64];
  char pt_vs_en_n[N_CLASS][N_TRIG][64];
  char y_vs_x_n[N_CLASS][N_TRIG][64];
  char z_vs_eta_n[N_CLASS][N_TRIG][64];
  char z_vs_phi_n[N_CLASS][N_TRIG][64];
  char mass_vs_en_n[N_CLASS][N_TRIG][64];
  char mass_vs_pt_n[N_CLASS][N_TRIG][64];
  char mass_dist_n[N_CLASS][N_TRIG][64];
  char z_dist_n[N_CLASS][N_TRIG][64];

  char pt_vs_eta_t[N_CLASS][N_TRIG][256];
  char en_vs_eta_t[N_CLASS][N_TRIG][256];
  char pt_vs_phi_t[N_CLASS][N_TRIG][256];
  char en_vs_phi_t[N_CLASS][N_TRIG][256];
  char eta_vs_phi_t[N_CLASS][N_TRIG][256];
  char pt_vs_en_t[N_CLASS][N_TRIG][256];
  char y_vs_x_t[N_CLASS][N_TRIG][256];
  char z_vs_eta_t[N_CLASS][N_TRIG][256];
  char z_vs_phi_t[N_CLASS][N_TRIG][256];
  char mass_vs_en_t[N_CLASS][N_TRIG][256];
  char mass_vs_pt_t[N_CLASS][N_TRIG][256];
  char mass_dist_t[N_CLASS][N_TRIG][256];
  char z_dist_t[N_CLASS][N_TRIG][256];

  char pt_rdist_n[N_CLASS][N_TRIG][MAXRUNS][64];
  char en_rdist_n[N_CLASS][N_TRIG][MAXRUNS][64];
  char eta_rdist_n[N_CLASS][N_TRIG][MAXRUNS][64];
  char phi_rdist_n[N_CLASS][N_TRIG][MAXRUNS][64];
  char mass_rdist_n[N_CLASS][N_TRIG][MAXRUNS][64];
  char z_rdist_n[N_CLASS][N_TRIG][MAXRUNS][64];

  char pt_rdist_n_tmp[N_CLASS][N_TRIG][MAXRUNS][64];
  char en_rdist_n_tmp[N_CLASS][N_TRIG][MAXRUNS][64];
  char eta_rdist_n_tmp[N_CLASS][N_TRIG][MAXRUNS][64];
  char phi_rdist_n_tmp[N_CLASS][N_TRIG][MAXRUNS][64];
  char mass_rdist_n_tmp[N_CLASS][N_TRIG][MAXRUNS][64];
  char z_rdist_n_tmp[N_CLASS][N_TRIG][MAXRUNS][64];

  char RP_select_t[32];
  if(!strcmp(env->RPselect,"")) strcpy(RP_select_t,"");
  else sprintf(RP_select_t," --- %s boolean only",env->RPselect);

  Float_t low_mass,high_mass;

  for(Int_t t=0; t<N_TRIG; t++)
  {
    for(Int_t c=0; c<N_CLASS; c++)
    {
      low_mass=0; // default
      high_mass=1; // default
      if(c==ev->Idx("thr")) 
      {
        low_mass=0.7; // to match cutoff in EventClass.cxx
        high_mass=6;
      }
      else if(c==ev->Idx("etm")) 
      {
        low_mass=0.3;
      }
      /*
      else if(c==ev->Idx("jps"))
      {
        low_mass=0;
        high_mass=6;
      };
      */

      sprintf(pt_vs_eta_n[c][t],"%s_%s_pt_vs_eta",T->Name(t),ev->Name(c));
      sprintf(en_vs_eta_n[c][t],"%s_%s_en_vs_eta",T->Name(t),ev->Name(c));
      sprintf(pt_vs_phi_n[c][t],"%s_%s_pt_vs_phi",T->Name(t),ev->Name(c));
      sprintf(en_vs_phi_n[c][t],"%s_%s_en_vs_phi",T->Name(t),ev->Name(c));
      sprintf(eta_vs_phi_n[c][t],"%s_%s_eta_vs_phi",T->Name(t),ev->Name(c));
      sprintf(pt_vs_en_n[c][t],"%s_%s_pt_vs_en",T->Name(t),ev->Name(c));
      sprintf(y_vs_x_n[c][t],"%s_%s_y_vs_x",T->Name(t),ev->Name(c));
      sprintf(z_vs_eta_n[c][t],"%s_%s_z_vs_eta",T->Name(t),ev->Name(c));
      sprintf(z_vs_phi_n[c][t],"%s_%s_z_vs_phi",T->Name(t),ev->Name(c));
      sprintf(mass_vs_en_n[c][t],"%s_%s_mass_vs_en",T->Name(t),ev->Name(c));
      sprintf(mass_vs_pt_n[c][t],"%s_%s_mass_vs_pt",T->Name(t),ev->Name(c));
      sprintf(mass_dist_n[c][t],"%s_%s_mass_dist",T->Name(t),ev->Name(c));
      sprintf(z_dist_n[c][t],"%s_%s_z_dist",T->Name(t),ev->Name(c));

      sprintf(pt_vs_eta_t[c][t],"%s %s --- p_{T} vs. #eta%s;#eta;p_{T}",
        T->Name(t),ev->Title(c),RP_select_t);
      sprintf(en_vs_eta_t[c][t],"%s %s --- E vs. #eta%s;#eta;E",
        T->Name(t),ev->Title(c),RP_select_t);
      sprintf(pt_vs_phi_t[c][t],"%s %s --- p_{T} vs. #phi%s;#phi;p_{T}",
        T->Name(t),ev->Title(c),RP_select_t);
      sprintf(en_vs_phi_t[c][t],"%s %s --- E vs. #phi%s;#phi;E",
        T->Name(t),ev->Title(c),RP_select_t);
      sprintf(eta_vs_phi_t[c][t],"%s %s --- #eta vs. #phi%s;#phi;#eta",
        T->Name(t),ev->Title(c),RP_select_t);
      sprintf(pt_vs_en_t[c][t],"%s %s --- p_{T} vs. E%s;E;p_{T}",
        T->Name(t),ev->Title(c),RP_select_t);
      sprintf(y_vs_x_t[c][t],"%s %s --- y vs. x%s;x;y",
        T->Name(t),ev->Title(c),RP_select_t);

      //printf("here\n");
      sprintf(z_vs_eta_t[c][t],"%s %s --- Z vs. #eta (%s cuts w/o Z-cut)%s;#eta;Z",
        T->Name(t),ev->Title(c),ev->Title(c),RP_select_t);
      sprintf(z_vs_phi_t[c][t],"%s %s --- Z vs. #phi (%s cuts w/o Z-cut)%s;#phi;Z",
        T->Name(t),ev->Title(c),ev->Title(c),RP_select_t);
      sprintf(mass_vs_en_t[c][t],"%s %s --- M vs. E (%s cuts w/o M-cut)%s;E;M",
        T->Name(t),ev->Title(c),ev->Title(c),RP_select_t);
      sprintf(mass_vs_pt_t[c][t],"%s %s --- M vs. p_{T} (%s cuts w/o M-cut)%s;p_{T};M",
        T->Name(t),ev->Title(c),ev->Title(c),RP_select_t);
      //printf("VVVVV\n");
      sprintf(mass_dist_t[c][t],"%s %s --- M distribution (%s cuts w/o M-cut)%s;M",
        T->Name(t),ev->Title(c),ev->Title(c),RP_select_t);
      //printf("%s\n",mass_dist_t[c][t]);
      sprintf(z_dist_t[c][t],"%s %s --- Z distribution (%s cuts w/o Z-cut)%s;Z",
        T->Name(t),ev->Title(c),ev->Title(c),RP_select_t);
      //printf("^^^^^\n");

      pt_vs_eta[c][t] = new TH2D(pt_vs_eta_n[c][t],pt_vs_eta_t[c][t],
        NBINS,Eta_min,Eta_max,NBINS,Pt_min,Pt_max);
      en_vs_eta[c][t] = new TH2D(en_vs_eta_n[c][t],en_vs_eta_t[c][t],
        NBINS,Eta_min,Eta_max,NBINS,E12_min,E12_max);
      pt_vs_phi[c][t] = new TH2D(pt_vs_phi_n[c][t],pt_vs_phi_t[c][t],
        NBINS,Phi_min,Phi_max,NBINS,Pt_min,Pt_max);
      en_vs_phi[c][t] = new TH2D(en_vs_phi_n[c][t],en_vs_phi_t[c][t],
        NBINS,Phi_min,Phi_max,NBINS,E12_min,E12_max);
      eta_vs_phi[c][t] = new TH2D(eta_vs_phi_n[c][t],eta_vs_phi_t[c][t],
        NBINS,Phi_min,Phi_max,NBINS,Eta_min,Eta_max);
      pt_vs_en[c][t] = new TH2D(pt_vs_en_n[c][t],pt_vs_en_t[c][t],
        NBINS,E12_min,E12_max,NBINS,Pt_min,Pt_max);
      y_vs_x[c][t] = new TH2D(y_vs_x_n[c][t],y_vs_x_t[c][t],
        NBINS,-110,110,NBINS,-110,110);
      z_vs_eta[c][t] = new TH2D(z_vs_eta_n[c][t],z_vs_eta_t[c][t],
        NBINS,Eta_min,Eta_max,NBINS,0,1);
      z_vs_phi[c][t] = new TH2D(z_vs_phi_n[c][t],z_vs_phi_t[c][t],
        NBINS,Phi_min,Phi_max,NBINS,0,1);
      mass_vs_en[c][t] = new TH2D(mass_vs_en_n[c][t],mass_vs_en_t[c][t],
        NBINS,E12_min,E12_max,NBINS,low_mass,high_mass);
      mass_vs_pt[c][t] = new TH2D(mass_vs_pt_n[c][t],mass_vs_pt_t[c][t],
        NBINS,Pt_min,Pt_max,NBINS,low_mass,high_mass);

      z_dist[c][t] = new TH1D(z_dist_n[c][t],z_dist_t[c][t],NBINS,0,1);
      mass_dist[c][t] = new TH1D(mass_dist_n[c][t],mass_dist_t[c][t],NBINS,low_mass,high_mass);

      //printf("----- c=%d t=%d %s %s\n",c,t,ev->Name(c),T->Name(t));
      for(Int_t ru=0; ru<MAXRUNS; ru++)
      {
        sprintf(pt_rdist_n[c][t][ru],"%s_%s_pt_rdist",T->Name(t),ev->Name(c));
        sprintf(en_rdist_n[c][t][ru],"%s_%s_en_rdist",T->Name(t),ev->Name(c));
        sprintf(eta_rdist_n[c][t][ru],"%s_%s_eta_rdist",T->Name(t),ev->Name(c));
        sprintf(phi_rdist_n[c][t][ru],"%s_%s_phi_rdist",T->Name(t),ev->Name(c));
        sprintf(mass_rdist_n[c][t][ru],"%s_%s_mass_rdist",T->Name(t),ev->Name(c));
        sprintf(z_rdist_n[c][t][ru],"%s_%s_z_rdist",T->Name(t),ev->Name(c));

        sprintf(pt_rdist_n_tmp[c][t][ru],"%s_%d",pt_rdist_n[c][t][ru],ru);
        sprintf(en_rdist_n_tmp[c][t][ru],"%s_%d",en_rdist_n[c][t][ru],ru);
        sprintf(eta_rdist_n_tmp[c][t][ru],"%s_%d",eta_rdist_n[c][t][ru],ru);
        sprintf(phi_rdist_n_tmp[c][t][ru],"%s_%d",phi_rdist_n[c][t][ru],ru);
        sprintf(mass_rdist_n_tmp[c][t][ru],"%s_%d",mass_rdist_n[c][t][ru],ru);
        sprintf(z_rdist_n_tmp[c][t][ru],"%s_%d",z_rdist_n[c][t][ru],ru);

        pt_rdist[c][t][ru] = new TH1D(pt_rdist_n_tmp[c][t][ru],pt_rdist_n_tmp[c][t][ru],
          NBINS_RDIST,Pt_min,Pt_max);
        en_rdist[c][t][ru] = new TH1D(en_rdist_n_tmp[c][t][ru],en_rdist_n_tmp[c][t][ru],
          NBINS_RDIST,E12_min,E12_max);
        eta_rdist[c][t][ru] = new TH1D(eta_rdist_n_tmp[c][t][ru],eta_rdist_n_tmp[c][t][ru],
          NBINS_RDIST,Eta_min,Eta_max);
        phi_rdist[c][t][ru] = new TH1D(phi_rdist_n_tmp[c][t][ru],phi_rdist_n_tmp[c][t][ru],
          NBINS_RDIST,Phi_min,Phi_max);
        mass_rdist[c][t][ru] = new TH1D(mass_rdist_n_tmp[c][t][ru],mass_rdist_n_tmp[c][t][ru],
          NBINS_RDIST,low_mass,high_mass);
        z_rdist[c][t][ru] = new TH1D(z_rdist_n_tmp[c][t][ru],z_rdist_n_tmp[c][t][ru],
          NBINS_RDIST,0,1);

      };
    };
  };


  // energy-dependent mass distributions for pions 
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
      mass_dist_for_enbin[t][ee] = 
        new TH1D(mass_dist_for_enbin_n[t][ee],mass_dist_for_enbin_t[t][ee],NBINS,0,1);
    };
  };

  // pt-dependent mass distributions for pions
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
      mass_dist_for_ptbin[t][pp] = 
        new TH1D(mass_dist_for_ptbin_n[t][pp],mass_dist_for_ptbin_t[t][pp],NBINS,0,1);
    };
  };


  // trigger distribution
  char trig_dist_t[64];
  sprintf(trig_dist_t,"Trigger Counts%s",RP_select_t);
  TH1D * trig_dist = new TH1D("trig_dist",trig_dist_t,N_TRIG,0,N_TRIG);
  for(Int_t t=0; t<N_TRIG; t++) trig_dist->GetXaxis()->SetBinLabel(t+1,T->Name(t));

  // trigger mix for FMS and RP
  Int_t N_RP_tmp = trg_bool->NBOOL;
  const Int_t N_RP = N_RP_tmp;
  printf("N_RP=%d\n",N_RP);

  TH2D * trig_fms_mix[N_CLASS]; // overlap matrix of FMS triggers
  TH2D * trig_rp_mix[N_CLASS]; // overlap matrix of RP triggers
  TH2D * trig_fmsrp_mix[N_CLASS]; // overlap matrix of FMS vs. RP triggers
                                  // (with extra column "n/a" for no RP trigger demand
                                 
  char trig_fms_mix_n[N_CLASS][64];
  char trig_rp_mix_n[N_CLASS][64];
  char trig_fmsrp_mix_n[N_CLASS][64];
  char trig_fms_mix_t[N_CLASS][256];
  char trig_rp_mix_t[N_CLASS][256];
  char trig_fmsrp_mix_t[N_CLASS][256];

  for(Int_t c=0; c<N_CLASS; c++)
  {
    sprintf(trig_fms_mix_n[c],"%s_trig_fms_mix",ev->Name(c));
    sprintf(trig_rp_mix_n[c],"%s_trig_rp_mix",ev->Name(c));
    sprintf(trig_fmsrp_mix_n[c],"%s_trig_fmsrp_mix",ev->Name(c));

    sprintf(trig_fms_mix_t[c],"%s FMS trigger overlap matrix",ev->Title(c));
    sprintf(trig_rp_mix_t[c],"%s RP trigger overlap matrix (using only OR of FMS triggers)",ev->Title(c));
    sprintf(trig_fmsrp_mix_t[c],"%s RP-FMS trigger overlap matrix",ev->Title(c));

    trig_fms_mix[c] = new TH2D(trig_fms_mix_n[c],trig_fms_mix_t[c],
      N_TRIG,0,N_TRIG,N_TRIG,0,N_TRIG);
    trig_rp_mix[c] = new TH2D(trig_rp_mix_n[c],trig_rp_mix_t[c],
      N_RP,0,N_RP,N_RP,0,N_RP);
    trig_fmsrp_mix[c] = new TH2D(trig_fmsrp_mix_n[c],trig_fmsrp_mix_t[c],
      N_RP+1,0,N_RP+1,N_TRIG,0,N_TRIG);

    for(Int_t t=0; t<N_TRIG; t++) 
    {
      trig_fms_mix[c]->GetXaxis()->SetBinLabel(t+1,T->Name(t));
      trig_fms_mix[c]->GetYaxis()->SetBinLabel(t+1,T->Name(t));
      trig_fmsrp_mix[c]->GetYaxis()->SetBinLabel(t+1,T->Name(t));
    };
    for(Int_t t=0; t<N_RP; t++)
    {
      trig_rp_mix[c]->GetXaxis()->SetBinLabel(t+1,trg_bool->Name(t));
      trig_rp_mix[c]->GetYaxis()->SetBinLabel(t+1,trg_bool->Name(t));
      trig_fmsrp_mix[c]->GetXaxis()->SetBinLabel(t+1,trg_bool->Name(t));
    };
    trig_fmsrp_mix[c]->GetXaxis()->SetBinLabel(N_RP+1,"n/a");
  };


  // fill histograms
  Int_t runnum_tmp=0;
  Int_t runcount=-1;
  Double_t pt_bc,en_bc,eta_bc,phi_bc,z_bc,mass_bc;
  Int_t pt_bn,en_bn,eta_bn,phi_bn,z_bn,mass_bn;

  Double_t evp;

  bbc->ev_canv->Clear();
  bbc->ev_canv->Print("bbc_evdisp.pdf(","pdf");;


  Int_t ENT = tr->GetEntries();
  //ENT = 1000; // uncomment to do a short loop for testing
  Int_t DrawLimit=1000; // how many bbc events to draw
  Int_t DrawCount=0;
  for(Int_t x=0; x<ENT; x++)
  {
    if((x%100000)==0) printf("filling histograms: %.2f%%\n",100*((Float_t)x)/((Float_t)ENT));
    tr->GetEntry(x);

    //kicked = RD->Kicked(runnum,bx);
    kicked = 0;

    // get new polarisation and check rellum
    // also fill kinematic vs. run plots
    if(runnum!=runnum_tmp)
    {
      /*
      b_pol = RD->BluePol(runnum);
      y_pol = RD->YellPol(runnum);
      isConsistent = RD->RellumConsistent(runnum);
      */
      b_pol = 0.5;
      y_pol = 0.5;
      isConsistent = 1;

      runcount++;
      printf(">>> %d <<<\n",runnum);
      for(Int_t t=0; t<N_TRIG; t++)
      {
        for(Int_t c=0; c<N_CLASS; c++)
        {
          sprintf(pt_rdist_n[c][t][runcount],"%s_%d",pt_rdist_n[c][t][runcount],runnum);
          sprintf(en_rdist_n[c][t][runcount],"%s_%d",en_rdist_n[c][t][runcount],runnum);
          sprintf(eta_rdist_n[c][t][runcount],"%s_%d",eta_rdist_n[c][t][runcount],runnum);
          sprintf(phi_rdist_n[c][t][runcount],"%s_%d",phi_rdist_n[c][t][runcount],runnum);
          sprintf(z_rdist_n[c][t][runcount],"%s_%d",z_rdist_n[c][t][runcount],runnum);
          sprintf(mass_rdist_n[c][t][runcount],"%s_%d",mass_rdist_n[c][t][runcount],runnum);

          pt_rdist[c][t][runcount]->SetName(pt_rdist_n[c][t][runcount]);
          en_rdist[c][t][runcount]->SetName(en_rdist_n[c][t][runcount]);
          eta_rdist[c][t][runcount]->SetName(eta_rdist_n[c][t][runcount]);
          phi_rdist[c][t][runcount]->SetName(phi_rdist_n[c][t][runcount]);
          z_rdist[c][t][runcount]->SetName(z_rdist_n[c][t][runcount]);
          mass_rdist[c][t][runcount]->SetName(mass_rdist_n[c][t][runcount]);

          pt_rdist[c][t][runcount]->SetTitle(pt_rdist_n[c][t][runcount]);
          en_rdist[c][t][runcount]->SetTitle(en_rdist_n[c][t][runcount]);
          eta_rdist[c][t][runcount]->SetTitle(eta_rdist_n[c][t][runcount]);
          phi_rdist[c][t][runcount]->SetTitle(phi_rdist_n[c][t][runcount]);
          z_rdist[c][t][runcount]->SetTitle(z_rdist_n[c][t][runcount]);
          mass_rdist[c][t][runcount]->SetTitle(mass_rdist_n[c][t][runcount]);
        };
      };
      runnum_tmp=runnum;
    }

    // rellum / pol cut
    if( kicked==0 && isConsistent==1 && b_pol>0 && y_pol>0)
    {
      // fill fms trigger plot
      for(Int_t t=0; t<N_TRIG; t++)
      {
        if(L2sum[1] & T->Mask(runnum,t,1) && trg_bool->Fired(env->RPselect))
          trig_dist->Fill(t);
      };

      // below here we have if statements for each event class; within each
      // if block, we have trigger loops, since it's more efficient to first pick the event and 
      // then loop through all triggers

      ev->SetKinematics(runnum,E12,Pt,Eta,Phi,M12,Z,N12);
      for(Int_t c=0; c<N_CLASS; c++)
      {
        // fill main kinematic correlation plots
        if(ev->Valid(c))
        {
          ev->FiducialGeom(Eta,Phi,0); // compute x and y coordinates
          //for(Int_t t=0; t<N_TRIG; t++)
          //{
            //if(L2sum[1] & T->Mask(runnum,t,1))
            //{
              //if(trg_bool->Fired(env->RPselect))
              //{
                /**************************************************************/
                // execute EVP test calculation
                bbc->UpdateEvent();
                if(bbc->QTN[0][0]>0 || bbc->QTN[1][0]>0)
                {
                  // only print the display if it's an FMSOR pi0
                  if(c==1)
                  {
                    bbc->DrawEvent();

                    // fill histograms
                    for(ew=0; ew<2; ew++)
                    {
                      for(sl=0; sl<2; sl++)
                      {
                        for(int qq=0; qq<bbc->QTN[ew][sl]; qq++)
                        {
                          adc[ew][sl][bbc->Idx[ew][sl][qq] - 1]->Fill(bbc->ADC[ew][sl][qq]);
                          tac[ew][sl][bbc->Idx[ew][sl][qq] - 1]->Fill(bbc->TAC[ew][sl][qq]);
                          adc_vs_tac[ew][sl][bbc->Idx[ew][sl][qq] - 1]->Fill(bbc->TAC[ew][sl][qq],bbc->ADC[ew][sl][qq]);
                        };
                      };
                    };


                    // print event to pdf
                    if(DrawCount<DrawLimit)
                    {
                      bbc->ev_canv->Print("bbc_evdisp.pdf","pdf");
                      DrawCount++;
                    };
                    for(ew=0; ew<2; ew++)
                    {
                      for(sl=0; sl<2; sl++)
                      {
                        //printf("ew=%d sl=%d evp=%.2f\n",ew,sl,bbc->EVP[ew][sl]*180.0/3.1415);
                        ou_evp[ew][sl]=bbc->EVP[ew][sl];
                        ou_evp_cor[ew][sl]=bbc->EVP_cor[ew][sl];
                        esum[ew][sl]=bbc->esum[ew][sl];
                        Xflow[ew][sl]=bbc->Xflow[ew][sl];
                        Yflow[ew][sl]=bbc->Yflow[ew][sl];
                        Xflow_cor[ew][sl]=bbc->Xflow_cor[ew][sl];
                        Yflow_cor[ew][sl]=bbc->Yflow_cor[ew][sl];
                        sigma_x[ew][sl]=bbc->sigma_x[ew][sl];
                        sigma_y[ew][sl]=bbc->sigma_y[ew][sl];
                        sigma_xy[ew][sl]=bbc->sigma_xy[ew][sl];
                        sigma_min[ew][sl]=bbc->sigma_min[ew][sl];
                        sigma_max[ew][sl]=bbc->sigma_max[ew][sl];
                        sigma_theta[ew][sl]=bbc->sigma_theta[ew][sl];
                        ou_H=bbc->IsHorizontal();
                        ou_V=bbc->IsVertical();
                      };
                      ou_qtn[ew]=(bbc->QTN[ew][0]);
                    };
                    outtr->Fill();
                  };
                };

                /**************************************************************/


                /*
                pt_vs_eta[c][t]->Fill(Eta,Pt);
                en_vs_eta[c][t]->Fill(Eta,E12);
                pt_vs_phi[c][t]->Fill(Phi,Pt);
                en_vs_phi[c][t]->Fill(Phi,E12);
                eta_vs_phi[c][t]->Fill(Phi,Eta);
                pt_vs_en[c][t]->Fill(E12,Pt);
                y_vs_x[c][t]->Fill(ev->Xd,ev->Yd);

                pt_rdist[c][t][runcount]->Fill(Pt);
                en_rdist[c][t][runcount]->Fill(E12);
                eta_rdist[c][t][runcount]->Fill(Eta);
                phi_rdist[c][t][runcount]->Fill(Phi);
                */
              //};

              // fill trigger overlap matrices
              if(enableOverlap)
              {
                for(Int_t tt=0; tt<N_TRIG; tt++)
                {
                  if(L2sum[1] & T->Mask(runnum,tt,1))
                    trig_fms_mix[c]->Fill(t,tt);
                };
                for(Int_t r=0; r<N_RP; r++)
                {
                  if(trg_bool->Fired(r))
                  {
                    trig_fmsrp_mix[c]->Fill(r,t);

                    // only fill trig_rp_mix for OR of FMS triggers
                    if(!strcmp(T->Name(t),"All"))
                    {
                      for(Int_t rr=0; rr<N_RP; rr++)
                      {
                        if(trg_bool->Fired(rr)) trig_rp_mix[c]->Fill(r,rr);
                      };
                    };
                  };
                };
                trig_fmsrp_mix[c]->Fill(N_RP,t); // fill "n/a" column
              };
            //};
          //};
        };

        // fill mass plots
        /*
        if(c!=ev->Idx("sph") && ev->ValidWithoutMcut(c))
        {
          for(Int_t t=0; t<N_TRIG; t++)
          {
            if(L2sum[1] & T->Mask(runnum,t,1))
            {
              if(trg_bool->Fired(env->RPselect))
              {
                mass_dist[c][t]->Fill(M12);
                mass_vs_en[c][t]->Fill(E12,M12);
                mass_vs_pt[c][t]->Fill(Pt,M12);
                mass_rdist[c][t][runcount]->Fill(M12);
                // fill kinematic-dependent mass distributions for pions
                if(c==ev->Idx("pi0"))
                {
                  for(Int_t ee=0; ee<10; ee++)
                  {
                    if(E12>=(ee*10) && E12<((ee+1)*10)) mass_dist_for_enbin[t][ee]->Fill(M12);
                  };
                  for(Int_t pp=0; pp<10; pp++)
                  {
                    if(Pt>=pp && Pt<(pp+1)) mass_dist_for_ptbin[t][pp]->Fill(M12);
                  };
                };
              };
            };
          };
        };

        // fill z plots
        if((c==ev->Idx("pi0") || c==ev->Idx("etm") ) && ev->ValidWithoutZcut(c))
        {
          for(Int_t t=0; t<N_TRIG; t++)
          {
            if(L2sum[1] & T->Mask(runnum,t,1))
            {
              if(trg_bool->Fired(env->RPselect))
              {
                z_vs_eta[c][t]->Fill(Eta,Z);
                z_vs_phi[c][t]->Fill(Phi,Z);
                z_dist[c][t]->Fill(Z);
                z_rdist[c][t][runcount]->Fill(Z);
              };
            };
          };
        };
        */
      };
    };
  };

  bbc->ev_canv->Clear();
  bbc->ev_canv->Print("bbc_evdisp.pdf)","pdf");;

  
  // histogram canvases
  TCanvas * adc_canv[2][2];
  TCanvas * tac_canv[2][2];
  TCanvas * adc_vs_tac_canv[2][2];
  TString adc_canv_n[2][2];
  TString tac_canv_n[2][2];
  TString adc_vs_tac_canv_n[2][2];
  for(ew=0; ew<2; ew++)
  {
    for(sl=0; sl<2; sl++)
    {
      adc_canv_n[ew][sl] = Form("adc_canv_%s_%s",ew_str[ew].Data(),sl_str[sl].Data());
      tac_canv_n[ew][sl] = Form("tac_canv_%s_%s",ew_str[ew].Data(),sl_str[sl].Data());
      adc_vs_tac_canv_n[ew][sl] = Form("adc_vs_tac_canv_%s_%s",ew_str[ew].Data(),sl_str[sl].Data());

      adc_canv[ew][sl] = new TCanvas(adc_canv_n[ew][sl].Data(),adc_canv_n[ew][sl].Data(),1000,1000);
      tac_canv[ew][sl] = new TCanvas(tac_canv_n[ew][sl].Data(),tac_canv_n[ew][sl].Data(),1000,1000);
      adc_vs_tac_canv[ew][sl] = new TCanvas(adc_vs_tac_canv_n[ew][sl].Data(),adc_vs_tac_canv_n[ew][sl].Data(),1000,1000);

      adc_canv[ew][sl]->Divide(4,4);
      tac_canv[ew][sl]->Divide(4,4);
      adc_vs_tac_canv[ew][sl]->Divide(4,4);

      for(chan=0; chan<16; chan++)
      {
        adc_canv[ew][sl]->GetPad(chan+1)->SetLogy();
        tac_canv[ew][sl]->GetPad(chan+1)->SetLogy();
        adc_vs_tac_canv[ew][sl]->GetPad(chan+1)->SetLogz();
        adc_canv[ew][sl]->cd(chan+1); 
        adc[ew][sl][chan]->Draw();
        tac_canv[ew][sl]->cd(chan+1); 
        tac[ew][sl][chan]->Draw();
        adc_vs_tac_canv[ew][sl]->cd(chan+1); 
        adc_vs_tac[ew][sl][chan]->Draw("colz");
      };
    };
  };


  // write output
  outfile->cd();
  outtr->Write("tr");
  for(int ew=0; ew<2; ew++) { for(sl=0; sl<2; sl++) { adc_canv[ew][sl]->Write(); }; };
  for(int ew=0; ew<2; ew++) { for(sl=0; sl<2; sl++) { tac_canv[ew][sl]->Write(); }; };
  for(int ew=0; ew<2; ew++) { for(sl=0; sl<2; sl++) { adc_vs_tac_canv[ew][sl]->Write(); }; };
  


  // build TObjArrays
  /*
  TObjArray * pt_vs_eta_arr[N_CLASS];
  TObjArray * en_vs_eta_arr[N_CLASS];
  TObjArray * pt_vs_phi_arr[N_CLASS];
  TObjArray * en_vs_phi_arr[N_CLASS];
  TObjArray * eta_vs_phi_arr[N_CLASS];
  TObjArray * pt_vs_en_arr[N_CLASS];
  TObjArray * y_vs_x_arr[N_CLASS];
  TObjArray * z_vs_eta_arr[N_CLASS];
  TObjArray * z_vs_phi_arr[N_CLASS];
  TObjArray * mass_vs_en_arr[N_CLASS];
  TObjArray * mass_vs_pt_arr[N_CLASS];
  TObjArray * mass_dist_arr[N_CLASS];
  TObjArray * z_dist_arr[N_CLASS];
  TObjArray * mass_dist_for_enbin_arr[10];
  TObjArray * mass_dist_for_ptbin_arr[10];

  TObjArray * pt_rdist_arr[N_CLASS][MAXRUNS];
  TObjArray * en_rdist_arr[N_CLASS][MAXRUNS];
  TObjArray * eta_rdist_arr[N_CLASS][MAXRUNS];
  TObjArray * phi_rdist_arr[N_CLASS][MAXRUNS];
  TObjArray * z_rdist_arr[N_CLASS][MAXRUNS];
  TObjArray * mass_rdist_arr[N_CLASS][MAXRUNS];

  char pt_vs_eta_arr_n[N_CLASS][32];
  char en_vs_eta_arr_n[N_CLASS][32];
  char pt_vs_phi_arr_n[N_CLASS][32];
  char en_vs_phi_arr_n[N_CLASS][32];
  char eta_vs_phi_arr_n[N_CLASS][32];
  char pt_vs_en_arr_n[N_CLASS][32];
  char y_vs_x_arr_n[N_CLASS][32];
  char z_vs_eta_arr_n[N_CLASS][32];
  char z_vs_phi_arr_n[N_CLASS][32];
  char mass_vs_en_arr_n[N_CLASS][32];
  char mass_vs_pt_arr_n[N_CLASS][32];
  char mass_dist_arr_n[N_CLASS][32];
  char z_dist_arr_n[N_CLASS][32];
  char mass_dist_for_enbin_arr_n[10][32];
  char mass_dist_for_ptbin_arr_n[10][32];

  char pt_rdist_arr_n[N_CLASS][MAXRUNS][32];
  char en_rdist_arr_n[N_CLASS][MAXRUNS][32];
  char eta_rdist_arr_n[N_CLASS][MAXRUNS][32];
  char phi_rdist_arr_n[N_CLASS][MAXRUNS][32];
  char z_rdist_arr_n[N_CLASS][MAXRUNS][32];
  char mass_rdist_arr_n[N_CLASS][MAXRUNS][32];


  for(Int_t c=0; c<N_CLASS; c++)
  {
    sprintf(pt_vs_eta_arr_n[c],"%s_pt_vs_eta_arr",ev->Name(c));
    sprintf(en_vs_eta_arr_n[c],"%s_en_vs_eta_arr",ev->Name(c));
    sprintf(pt_vs_phi_arr_n[c],"%s_pt_vs_phi_arr",ev->Name(c));
    sprintf(en_vs_phi_arr_n[c],"%s_en_vs_phi_arr",ev->Name(c));
    sprintf(eta_vs_phi_arr_n[c],"%s_eta_vs_phi_arr",ev->Name(c));
    sprintf(pt_vs_en_arr_n[c],"%s_pt_vs_en_arr",ev->Name(c));
    sprintf(y_vs_x_arr_n[c],"%s_y_vs_x_arr",ev->Name(c));
    sprintf(z_vs_eta_arr_n[c],"%s_z_vs_eta_arr",ev->Name(c));
    sprintf(z_vs_phi_arr_n[c],"%s_z_vs_phi_arr",ev->Name(c));
    sprintf(mass_vs_en_arr_n[c],"%s_mass_vs_en_arr",ev->Name(c));
    sprintf(mass_vs_pt_arr_n[c],"%s_mass_vs_pt_arr",ev->Name(c));
    sprintf(mass_dist_arr_n[c],"%s_mass_dist_arr",ev->Name(c));
    sprintf(z_dist_arr_n[c],"%s_z_dist_arr",ev->Name(c));

    pt_vs_eta_arr[c] = new TObjArray();
    en_vs_eta_arr[c] = new TObjArray();
    pt_vs_phi_arr[c] = new TObjArray();
    en_vs_phi_arr[c] = new TObjArray();
    eta_vs_phi_arr[c] = new TObjArray();
    pt_vs_en_arr[c] = new TObjArray();
    y_vs_x_arr[c] = new TObjArray();
    z_vs_eta_arr[c] = new TObjArray();
    z_vs_phi_arr[c] = new TObjArray();
    mass_vs_en_arr[c] = new TObjArray();
    mass_vs_pt_arr[c] = new TObjArray();
    mass_dist_arr[c] = new TObjArray();
    z_dist_arr[c] = new TObjArray();

    for(Int_t ru=0; ru<MAXRUNS; ru++)
    {
      sprintf(pt_rdist_arr_n[c][ru],"%s_pt_rdist_arr_%d",ev->Name(c),ru);
      sprintf(en_rdist_arr_n[c][ru],"%s_en_rdist_arr_%d",ev->Name(c),ru);
      sprintf(eta_rdist_arr_n[c][ru],"%s_eta_rdist_arr_%d",ev->Name(c),ru);
      sprintf(phi_rdist_arr_n[c][ru],"%s_phi_rdist_arr_%d",ev->Name(c),ru);
      sprintf(z_rdist_arr_n[c][ru],"%s_z_rdist_arr_%d",ev->Name(c),ru);
      sprintf(mass_rdist_arr_n[c][ru],"%s_mass_rdist_arr_%d",ev->Name(c),ru);
      pt_rdist_arr[c][ru] = new TObjArray();
      en_rdist_arr[c][ru] = new TObjArray();
      eta_rdist_arr[c][ru] = new TObjArray();
      phi_rdist_arr[c][ru] = new TObjArray();
      z_rdist_arr[c][ru] = new TObjArray();
      mass_rdist_arr[c][ru] = new TObjArray();
    };
  };
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
      y_vs_x_arr[c]->AddLast(y_vs_x[c][t]);
      z_vs_eta_arr[c]->AddLast(z_vs_eta[c][t]);
      z_vs_phi_arr[c]->AddLast(z_vs_phi[c][t]);
      mass_vs_en_arr[c]->AddLast(mass_vs_en[c][t]);
      mass_vs_pt_arr[c]->AddLast(mass_vs_pt[c][t]);
      mass_dist_arr[c]->AddLast(mass_dist[c][t]);
      z_dist_arr[c]->AddLast(z_dist[c][t]);
      for(Int_t ru=0; ru<=runcount; ru++)
      {
        pt_rdist_arr[c][ru]->AddLast(pt_rdist[c][t][ru]);
        en_rdist_arr[c][ru]->AddLast(en_rdist[c][t][ru]);
        eta_rdist_arr[c][ru]->AddLast(eta_rdist[c][t][ru]);
        phi_rdist_arr[c][ru]->AddLast(phi_rdist[c][t][ru]);
        z_rdist_arr[c][ru]->AddLast(z_rdist[c][t][ru]);
        mass_rdist_arr[c][ru]->AddLast(mass_rdist[c][t][ru]);
      };
    };
    for(Int_t k=0; k<10; k++)
    {
      mass_dist_for_enbin_arr[k]->AddLast(mass_dist_for_enbin[t][k]);
      mass_dist_for_ptbin_arr[k]->AddLast(mass_dist_for_ptbin[t][k]);
    };
  };


  // write output
  char outfilename[256];
  char RP_suffix[16];
  sscanf(infile_name,"RedOutput%s",outfilename);
  if(!strcmp(env->RPselect,"")) strcpy(RP_suffix,"");
  else sprintf(RP_suffix,"_%s",env->RPselect);
  sprintf(outfilename,"diagset/diag%s.%s",RP_suffix,outfilename);
  TFile * outfile = new TFile(outfilename,"RECREATE");
  outfile->cd();

  trig_dist->Write();

  if(enableOverlap)
  {
    outfile->mkdir("overlap_matrices");
    outfile->cd("overlap_matrices");
    for(Int_t c=0; c<N_CLASS; c++) trig_fms_mix[c]->Write();
    for(Int_t c=0; c<N_CLASS; c++) trig_rp_mix[c]->Write();
    for(Int_t c=0; c<N_CLASS; c++) trig_fmsrp_mix[c]->Write();
  };
  outfile->cd();
  outfile->mkdir("rdists");
  outfile->cd("rdists");
  for(Int_t c=0; c<N_CLASS; c++)
  {
    for(Int_t ru=0; ru<=runcount; ru++)
    {
      //printf("ru=%d c=%d %p\n",ru,c,(void*)pt_rdist_arr_n[c][ru]);
      //printf("%s\n",pt_rdist_arr_n[c][ru]);
      pt_rdist_arr[c][ru]->Write(pt_rdist_arr_n[c][ru],TObject::kSingleKey);
      en_rdist_arr[c][ru]->Write(en_rdist_arr_n[c][ru],TObject::kSingleKey);
      eta_rdist_arr[c][ru]->Write(eta_rdist_arr_n[c][ru],TObject::kSingleKey);
      phi_rdist_arr[c][ru]->Write(phi_rdist_arr_n[c][ru],TObject::kSingleKey);
      z_rdist_arr[c][ru]->Write(z_rdist_arr_n[c][ru],TObject::kSingleKey);
      mass_rdist_arr[c][ru]->Write(mass_rdist_arr_n[c][ru],TObject::kSingleKey);
    };
  };
  outfile->cd();
  for(Int_t c=0; c<N_CLASS; c++)
  {
    pt_vs_eta_arr[c]->Write(pt_vs_eta_arr_n[c],TObject::kSingleKey);
    en_vs_eta_arr[c]->Write(en_vs_eta_arr_n[c],TObject::kSingleKey);
    pt_vs_phi_arr[c]->Write(pt_vs_phi_arr_n[c],TObject::kSingleKey);
    en_vs_phi_arr[c]->Write(en_vs_phi_arr_n[c],TObject::kSingleKey);
    eta_vs_phi_arr[c]->Write(eta_vs_phi_arr_n[c],TObject::kSingleKey);
    pt_vs_en_arr[c]->Write(pt_vs_en_arr_n[c],TObject::kSingleKey);
    y_vs_x_arr[c]->Write(y_vs_x_arr_n[c],TObject::kSingleKey);
    z_vs_eta_arr[c]->Write(z_vs_eta_arr_n[c],TObject::kSingleKey);
    z_vs_phi_arr[c]->Write(z_vs_phi_arr_n[c],TObject::kSingleKey);
    mass_vs_en_arr[c]->Write(mass_vs_en_arr_n[c],TObject::kSingleKey);
    mass_vs_pt_arr[c]->Write(mass_vs_pt_arr_n[c],TObject::kSingleKey);
    mass_dist_arr[c]->Write(mass_dist_arr_n[c],TObject::kSingleKey);
    z_dist_arr[c]->Write(z_dist_arr_n[c],TObject::kSingleKey);
  };
  for(Int_t k=0; k<10; k++)
  {
    mass_dist_for_enbin_arr[k]->Write(mass_dist_for_enbin_arr_n[k],TObject::kSingleKey);
  };
  for(Int_t k=0; k<10; k++)
  {
    mass_dist_for_ptbin_arr[k]->Write(mass_dist_for_ptbin_arr_n[k],TObject::kSingleKey);
  };
  */
};
