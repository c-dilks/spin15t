// compares RP trigger dependent asymmetries
//
// asym: A_Sigma, R_blue, R_yellow
//       A_TT, A_N_blue, A_N_yellow

void CompareRPasym(char * kinvar="en", char * evclass="pi0",
                   char * asym="A_N_blue", char * binselect="")
{
  char dir[32];
  strcpy(dir,"expedited");
  strcpy(dir,"spinpwg");

  const Int_t NRP = 10;
  enum rp_enum {kAll,kET,kIT,kSDE,kSDW,kEOR,kWOR,kEXOR,kWXOR,kDD};

  // draw switch
  Bool_t draw[NRP];
  draw[kAll] = 0;

  draw[kET] = 0;
  draw[kIT] = 0;

  draw[kSDE] = 0;
  draw[kSDW] = 0;

  draw[kEOR] = 1;
  draw[kWOR] = 0;

  draw[kDD] = 0;

  draw[kEXOR] = 0;
  draw[kWXOR] = 0;



  /////////////////////////

  char rp_name[NRP][16];
  strcpy(rp_name[kAll],"N");
  strcpy(rp_name[kET],"ET");
  strcpy(rp_name[kIT],"IT");
  strcpy(rp_name[kSDE],"SDE");
  strcpy(rp_name[kSDW],"SDW");
  strcpy(rp_name[kEOR],"EOR");
  strcpy(rp_name[kWOR],"WOR");
  strcpy(rp_name[kEXOR],"EXOR");
  strcpy(rp_name[kWXOR],"WXOR");
  strcpy(rp_name[kDD],"DD");

  Int_t rp_plot_color[NRP];
  rp_plot_color[kAll] = (Int_t) kRed;
  rp_plot_color[kET] = (Int_t) kOrange;
  rp_plot_color[kIT] = (Int_t) kCyan+2;
  rp_plot_color[kSDE] = (Int_t) kGreen+2;
  rp_plot_color[kSDW] = (Int_t) kBlue;
  rp_plot_color[kEOR] = (Int_t) kGreen+2;
  rp_plot_color[kWOR] = (Int_t) kBlue;
  rp_plot_color[kEXOR] = (Int_t) kGreen+2;
  rp_plot_color[kWXOR] = (Int_t) kBlue;
  rp_plot_color[kDD] = (Int_t) kBlack;

  Int_t A,Z;
  TString asym_str;
  if(!strcmp(asym,"A_Sigma")) { Z=0; A=3; asym_str="A_{#Sigma}";}
  else if(!strcmp(asym,"R_blue")) { Z=0; A=2; asym_str="R^{B}";}
  else if(!strcmp(asym,"R_yellow")) { Z=0; A=1; asym_str="R^{Y}";}
  else if(!strcmp(asym,"A_TT")) { Z=1; A=3; asym_str="A_{TT}";}
  else if(!strcmp(asym,"A_N_blue")) { Z=1; A=2; asym_str="A_{N}^{B}";}
  else if(!strcmp(asym,"A_N_yellow")) { Z=1; A=1; asym_str="A_{N}^{Y}";}
  else
  {
    fprintf(stderr,"ERROR: asym directory not valid\n");
    return;
  };
  
  char gr_n[32];
  TString kinvar_str;
  if(!strcmp(kinvar,"en")) 
  {
    sprintf(gr_n,"%s/en_dep_z%d_a%d_g0_p0",asym,Z,A);
    kinvar_str="E";
  }
  else if(!strcmp(kinvar,"pt")) 
  {
    sprintf(gr_n,"%s/pt_dep_z%d_a%d_g0_e0",asym,Z,A);
    kinvar_str="p_{T}";
  }
  else 
  {
    fprintf(stderr,"ERROR: kinvar not valid\n");
    return;
  };

  // QUICK TITLE FOR STAR COLLAB MEETING; CHANGE ME!!!
  TString evclass_str;
  if(!strcmp(evclass,"pi0")) evclass_str="#pi^{0}";
  else evclass_str=Form("%s",evclass);

  TFile * infile[NRP]; 
  char infile_n[NRP][256];
  TGraphErrors * gr[NRP];
  TMultiGraph * multi_gr = new TMultiGraph();
  char multi_gr_title[64];
  sprintf(multi_gr_title,"%s %s vs. %s",evclass_str.Data(),asym_str.Data(),kinvar_str.Data());
  if(strcmp(binselect,"")) sprintf(multi_gr_title,"%s (%s)",multi_gr_title,binselect);
  multi_gr->SetTitle(multi_gr_title);
  TLegend * leg = new TLegend(0.1,0.9,0.2,0.6);
  for(Int_t n=0; n<NRP; n++)
  {
    if(draw[n])
    {
      if(strcmp(binselect,"")) 
        sprintf(infile_n[n],"asym_plots/output_%s_%s_%s_%s/spin_%s.root",dir,kinvar,binselect,rp_name[n],evclass);
      else 
        sprintf(infile_n[n],"asym_plots/output_%s_%s_%s/spin_%s.root",dir,kinvar,rp_name[n],evclass);
      printf("%s\n",infile_n[n]);
      infile[n] = new TFile(infile_n[n],"READ");
      //infile[n]->cd(asym);
      gr[n] = (TGraphErrors*) infile[n]->Get(gr_n);
      printf("infile @ %p   gr[%d] @ %p\n",(void*)infile[n],n,(void*)gr[n]);
      gr[n]->SetMarkerColor(rp_plot_color[n]);
      gr[n]->SetLineColor(rp_plot_color[n]);
      gr[n]->SetMarkerSize(1.3);
      gr[n]->SetMarkerStyle(kFullCircle);
      gr[n]->SetLineWidth(2);
      leg->AddEntry(gr[n],rp_name[n],"LPE");
      multi_gr->Add(gr[n]);
    };
  };

  TCanvas * canv = new TCanvas("canv","canv",800,800);
  canv->SetGrid(1,1);
  multi_gr->Draw("ape");
  leg->Draw();
};
