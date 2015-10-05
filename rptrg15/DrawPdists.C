// draws probability of RP trigger vs. bXing for all RP triggers

void DrawPdists()
{
  const Int_t NTRIG = 11;
  const Int_t MAX_RUN = 616;
  TString config="production_pp200trans_2015";
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
  TFile * infile = new TFile("rptree.root","READ");
  TTree * tr = (TTree*) infile->Get("tr");
  int i;
  TString drawstr,cutstr;
  TFile * outfile = new TFile("pvsi_plots.root","RECREATE");
  TH1D * pvsi[NTRIG];
  TString pvsi_n[NTRIG];
  TString pvsi_t[NTRIG];
  for(i=0; i<NTRIG; i++)
  {
    pvsi_n[i] = trg_name[i]+"_prob_vs_i";
    pvsi_t[i] = "P_{"+trg_name[i]+"} vs. run index";
    pvsi[i] = new TH1D(pvsi_n[i].Data(),pvsi_t[i].Data(),MAX_RUN,0,MAX_RUN);
    drawstr = trg_name[i]+"*"+trg_name[i]+"_ps/(t*freq*10**6)";
    drawstr = drawstr+"*(type==\""+config+"\")";
    cout << drawstr << endl;
    tr->Project(pvsi_n[i],"i",drawstr);
    pvsi[i]->Fit("pol0","","",0,MAX_RUN);
    pvsi[i]->Write();
  };
};
