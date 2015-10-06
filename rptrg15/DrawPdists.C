// draws probability of RP trigger vs. bXing for all RP triggers

void DrawPdists()
{
  TFile * outfile = new TFile("pvsi_plots.root","RECREATE");


  // load RP tree
  int i;
  const Int_t NTRIG = 11;
  char config[64];
  strcpy(config,"production_pp200trans_2015");
  TString trg_name[NTRIG] = {
    "RP_SD",
    "RP_SDT",
    "RP_RPZMU",
    "RP_RPZE",
    "RP_ET",
    "RP_CP",
    "RP_RP2MU",
    "RP_RP2E",
    "RP_Zerobias",
    "RP_CPT2",
    "RP_CPEI"
  };
  TString trg_ps_name[NTRIG];
  for(i=0; i<NTRIG; i++) trg_ps_name[i]=trg_name[i]+"_ps";
  TFile * infile = new TFile("rptree.root","READ");
  TTree * tr = (TTree*) infile->Get("tr");

  // set up bXing dist histogram
  TFile * countsfile = new TFile("counts.root","READ");
  TTree * counts = (TTree*) countsfile->Get("sca");
  TH1D * bxdist = new TH1D("bxdist","bxdist",120,0,120);
  Int_t runnum_tmp=0;
  


  // RP branch addresses
  Int_t index;
  Int_t runnum;
  char ttype[64];
  Double_t trg[NTRIG];
  Double_t trg_ps[NTRIG];
  tr->SetBranchAddress("i",&index);
  tr->SetBranchAddress("type",ttype);
  tr->SetBranchAddress("runnum",&runnum);
  for(i=0; i<NTRIG; i++)
  {
    tr->SetBranchAddress(trg_name[i].Data(),&(trg[i]));
    tr->SetBranchAddress(trg_ps_name[i].Data(),&(trg_ps[i]));
  };


  // determine run index ranges
  Int_t MAX_RUN=0;
  Int_t MIN_RUN=100000;
  for(i=0; i<tr->GetEntries(); i++)
  {
    tr->GetEntry(i);
    if(!strcmp(ttype,config))
    {
      MIN_RUN=(index<MIN_RUN)?index:MIN_RUN;
      MAX_RUN=(index>MAX_RUN)?index:MAX_RUN;
    };
  };
  printf("MIN_RUN=%d MAX_RUN=%d\n",MIN_RUN,MAX_RUN);



  // define p (probability of trigger per bXing) vs. run index plots
  //TH1D * pvsi[NTRIG];
  TGraph * pvsi[NTRIG];
  TString pvsi_n[NTRIG];
  TString pvsi_t[NTRIG];
  Int_t pvsi_cnt[NTRIG];
  for(i=0; i<NTRIG; i++)
  {
    pvsi_n[i] = trg_name[i]+"_prob_vs_i";
    pvsi_t[i] = "P_{"+trg_name[i]+"} vs. run index";
    //pvsi[i] = new TH1D(pvsi_n[i].Data(),pvsi_t[i].Data(),MAX_RUN-MIN_RUN,MIN_RUN,MAX_RUN);
    pvsi[i] = new TGraph();
    pvsi[i]->SetName(pvsi_n[i].Data());
    pvsi[i]->SetTitle(pvsi_t[i].Data());
    pvsi[i]->SetMarkerStyle(kFullCircle);
    pvsi[i]->SetMarkerColor(kAzure);
    pvsi_cnt[i] = 0;
  };


  // define output tree
  TTree * ptr = new TTree("ptr","ptr");
  ptr->Branch("i",&index,"i/I");
  ptr->Branch("runnum",&runnum,"runnum/I");
  TString trg_typ[NTRIG];
  TString trg_ps_typ[NTRIG];
  TString p_name[NTRIG];
  TString p_typ[NTRIG];
  Double_t prob[NTRIG];
  for(i=0; i<NTRIG; i++)
  {
    trg_typ[i] = trg_name[i]+"/D";
    trg_ps_typ[i] = trg_ps_name[i]+"/D";
    p_name[i] = "Prob_"+trg_name[i];
    p_typ[i] = p_name[i]+"/D";
    ptr->Branch(trg_name[i].Data(),&(trg[i]),trg_typ[i].Data());
    ptr->Branch(trg_ps_name[i].Data(),&(trg_ps[i]),trg_ps_typ[i].Data());
    ptr->Branch(p_name[i].Data(),&(prob[i]),p_typ[i].Data());
  };


  // rp tree loop
  Double_t nbx=0;
  char counts_cut[128];
  for(i=0; i<tr->GetEntries(); i++)
  {
    tr->GetEntry(i);
    if(!strcmp(ttype,config))
    {
      if(runnum!=runnum_tmp)
      {
        runnum_tmp=runnum;
        bxdist->Reset();
        sprintf(counts_cut,"tot_bx*(runnum==%d && blue*yell!=0 && kicked==0)",runnum);
        counts->Project("bxdist","bx",counts_cut);
        nbx = bxdist->Integral();
        printf("runnum=%d nbx=%ld\n",runnum,nbx);
      };
      if(nbx>0)
      {
        for(int j=0; j<NTRIG; j++)
        {
          prob[j] = (trg[j]*trg_ps[j])/nbx;
          //pvsi[j]->SetBinContent(index+1,prob[j]); // trigger * prescale / no. bXings
          pvsi[j]->SetPoint(pvsi_cnt[j],index,prob[j]);
          (pvsi_cnt[j])++;
        };
        ptr->Fill();
      };
    };
  };


  
  // fit and write output
  outfile->cd();
  for(i=0; i<NTRIG; i++)
  {
    pvsi[i]->Fit("pol0","","",MIN_RUN,MAX_RUN);
    pvsi[i]->Write();
  };
  ptr->Write("ptr");
};
