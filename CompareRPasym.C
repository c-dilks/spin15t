// compares RP trigger dependent asymmetries
//
// asym: A_Sigma, R_blue, R_yellow
//       A_TT, A_N_blue, A_N_yellow

void CompareRPasym(char * kinvar="en", char * evclass="pi0",
                   char * asym="A_N_blue", char * binselect="")
{
  char dir[32];

  // strongest scint-based trg, selecting on inner MIPs
  strcpy(dir,"scint-mips");

  // strongest scint-based trg, excluding inner MIPs
  //strcpy(dir,"scint-nomips");

  // strongest scint-based trg, no MIP constraints
  //strcpy(dir,"scint");

  // TCU-like scint-based trigger; should be same as TCU-based trigger
  //strcpy(dir,"tcu-like");

  // TCU-based trigger
  //strcpy(dir,"tcu");

  //---------------------------------------------------------------

  // prepend dir name
  sprintf(dir,"FIXED2_%s",dir);


  //---------------------------------------------------------------


  const Int_t NRP = 10;
  enum rp_enum {kAll,kET,kIT,kSDE,kSDW,kEOR,kWOR,kEXOR,kWXOR,kDD};


  // draw switch
  const Int_t NPLOTS = 3;
  Bool_t draw[NPLOTS][NRP];
  TString png_name[NPLOTS];
  for(int i=0; i<NPLOTS; i++) for(int j=0; j<NRP; j++) draw[i][j] = false;
  png_name[0] = "EOR_v_WOR.png";
    draw[0][kAll]=true; draw[0][kEOR]=true; draw[0][kWOR]=true;
  ///*
  png_name[1] = "ET_v_IT.png";
    draw[1][kAll]=true; draw[1][kET]=true; draw[1][kIT]=true;
  png_name[2] = "SDE_v_SDW.png";
    draw[2][kAll]=true; draw[2][kSDE]=true; draw[2][kSDW]=true;
    //*/
   


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
  TGraphErrors * gr[NPLOTS][NRP];
  TMultiGraph * multi_gr[NPLOTS];
  for(int i=0; i<NPLOTS; i++) multi_gr[i] = new TMultiGraph();

  char multi_gr_title[64];
  sprintf(multi_gr_title,"%s %s vs. %s",evclass_str.Data(),asym_str.Data(),kinvar_str.Data());
  if(strcmp(binselect,"")) sprintf(multi_gr_title,"%s (%s)",multi_gr_title,binselect);
  for(int i=0; i<NPLOTS; i++) multi_gr[i]->SetTitle(multi_gr_title);

  TLegend * leg[NPLOTS];
  TString print_name[NPLOTS];
  for(int i=0; i<NPLOTS; i++)
  {
    leg[i] = new TLegend(0.1,0.9,0.2,0.6);
    for(Int_t n=0; n<NRP; n++)
    {
      if(draw[i][n])
      {
        if(strcmp(binselect,"")) 
        {
          sprintf(infile_n[n],"asym_plots/output_%s_%s_%s_%s/spin_%s.root",dir,kinvar,binselect,rp_name[n],evclass);
          print_name[i] = Form("asym_plots/output_%s_%s_%s_%s_%s",dir,kinvar,binselect,evclass,png_name[i].Data());
        }
        else 
        {
          sprintf(infile_n[n],"asym_plots/output_%s_%s_%s/spin_%s.root",dir,kinvar,rp_name[n],evclass);
          print_name[i] = Form("asym_plots/output_%s_%s_%s_%s",dir,kinvar,evclass,png_name[i].Data());
        };
        printf("%s\n",infile_n[n]);
        infile[n] = new TFile(infile_n[n],"READ");
        //infile[n]->cd(asym);
        gr[i][n] = (TGraphErrors*) infile[n]->Get(gr_n);
        printf("infile @ %p   gr[%d] @ %p\n",(void*)infile[n],n,(void*)gr[i][n]);
        gr[i][n]->SetMarkerColor(rp_plot_color[n]);
        gr[i][n]->SetLineColor(rp_plot_color[n]);
        gr[i][n]->SetMarkerSize(1.3);
        gr[i][n]->SetMarkerStyle(kFullCircle);
        gr[i][n]->SetLineWidth(2);
        leg[i]->AddEntry(gr[i][n],rp_name[n],"LPE");
        multi_gr[i]->Add(gr[i][n]);
      };
    };
  };

  TCanvas * canv = new TCanvas("canv","canv",800,800);
  for(int i=0; i<NPLOTS; i++)
  {
    canv->Clear();
    canv->SetGrid(1,1);
    multi_gr[i]->Draw("ape");
    leg[i]->Draw();
    canv->Print(print_name[i].Data(),"png");
  };
};
