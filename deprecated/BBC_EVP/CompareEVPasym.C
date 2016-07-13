// asym: A_Sigma, R_blue, R_yellow
//       A_TT, A_N_blue, A_N_yellow

void CompareEVPasym(TString direc="output", TString kinvar="en", TString evclass="pi0", TString asym="A_N_bluel")
{
  Bool_t debug=true;

  // event plane classes
  const Int_t NEVP = 3;
  enum evp_enum {kAll,kVer,kHor};
  TString evp_name[NEVP];
  evp_name[kVer] = "VER";
  evp_name[kHor] = "HOR";
  evp_name[kAll] = "ALL";


  // draw switch
  Bool_t draw[NEVP];
  int ne;
  for(ne=0; ne<NEVP; ne++) draw[ne] = false;
  draw[kAll]=true; 
  draw[kVer]=true; 
  draw[kHor]=true; 


  // colors
  Int_t evp_plot_color[NEVP];
  evp_plot_color[kAll] = (Int_t) kRed;
  evp_plot_color[kHor] = (Int_t) kGreen+2;
  evp_plot_color[kVer] = (Int_t) kMagenta;


  ///////////////////////////////////////////////////////

  
  // load binning
  gSystem->Load("src/RunInfo.so");
  Environ * env = new Environ();
  EventClass * ev = new EventClass();
  Int_t eta_bins0 = env->EtaBins; const Int_t eta_bins = eta_bins0;
  Int_t en_bins0 = env->EnBins; const Int_t en_bins = en_bins0;
  Int_t pt_bins0 = env->PtBins; const Int_t pt_bins = pt_bins0;
  if(debug) printf(" env->eta_bins=%d\n env->en_bins=%d\n env->pt_bins=%d\n",eta_bins,en_bins,pt_bins);


  // build spin.root filename
  TString spinfile_n[NEVP];
  for(ne=0; ne<NEVP; ne++) 
  {
    spinfile_n[ne] = "asym_plots/"+direc+"_"+evp_name[ne]+"/spin_"+evclass+".root";
    if(debug) printf("spinfile_n[%d]=%s\n",ne,spinfile_n[ne].Data());
  };


  // determine spin.root TDirectory and asym number and asym title
  Int_t A,Z;
  TString asym_str;
  if(asym=="A_Sigma") { Z=0; A=3; asym_str="A_{#Sigma}";}
  else if(asym=="R_blue") { Z=0; A=2; asym_str="R^{B}";}
  else if(asym=="R_yellow") { Z=0; A=1; asym_str="R^{Y}";}
  else if(asym=="A_TT") { Z=1; A=3; asym_str="A_{TT}";}
  else if(asym=="A_N_blue") { Z=1; A=2; asym_str="A_{N}^{B}";}
  else if(asym=="A_N_yellow") { Z=1; A=1; asym_str="A_{N}^{Y}";}
  else
  {
    fprintf(stderr,"ERROR: asym directory not valid\n");
    return;
  };
  if(debug) printf("%s A=%d Z=%d asym_str=%s\n",asym.Data(),A,Z,asym_str.Data());


  // evclass title info
  TString evclass_str = Form("%s",ev->Title(evclass.Data()));



  // determing NOTRO, the number of bins in the "other" kinematic variable (denoted "otro")
  // (i.e. if kinvar=pt, NOTRO=number of en bins)
  // NOTRO is then the number of TCanvases of plots which will be drawn
  Int_t NOTRO_tmp;
  TString kinvar_str;
  if(kinvar=="en") 
  {
    NOTRO_tmp = pt_bins; 
    kinvar_str = "E_{#gamma#gamma}";
  }
  else if(kinvar=="pt") 
  {
    NOTRO_tmp = en_bins;
    kinvar_str = "p_{T}";
  }
  else 
  {
    fprintf(stderr,"ERROR: kinvar should be en or pt\n");
    return;
  };
  const Int_t NOTRO = NOTRO_tmp;


  // build string to specify range of otro kinematic variable
  TString otro_str[NOTRO];
  int no;
  for(no=0; no<NOTRO; no++)
  {
    if(kinvar=="en") otro_str[no] = Form("p_{T}#in[%.2f,%.2f)",env->PtDiv(no),env->PtDiv(no+1));
    else if(kinvar=="pt") otro_str[no] = Form("E_{#gamma#gamma}#in[%.2f,%.2f)",env->EnDiv(no),env->EnDiv(no+1));
  };


  // build names of asym plots to retrieve from spin.root
  TString gr_n[NOTRO];
  for(no=0; no<NOTRO; no++)
  {
    if(kinvar=="en") gr_n[no] = Form("%s/en_dep_z%d_a%d_g0_p%d",asym.Data(),Z,A,no);
    else if(kinvar=="pt") gr_n[no] = Form("%s/pt_dep_z%d_a%d_g0_e%d",asym.Data(),Z,A,no);
  };



  // open spin.root file
  TFile * spinfile[NEVP];
  for(ne=0; ne<NEVP; ne++) 
  {
    if(draw[ne])
    {
      spinfile[ne] = new TFile(spinfile_n[ne].Data(),"READ");
    };
  };



  // build graph titles
  TGraphErrors * gr[NEVP][NOTRO];
  TMultiGraph * multi_gr[NOTRO];
  TString multi_gr_title[NOTRO];
  int np;
  for(no=0; no<NOTRO; no++)
  {
    multi_gr[no] = new TMultiGraph();
    multi_gr_title[no] = evclass_str+" "+asym_str+" vs. "+kinvar_str+" where "+otro_str[no];
    multi_gr[no]->SetTitle(multi_gr_title[no]);
  };


  // obtain graphs from spin.root files, add to multigraphs, and build legend
  for(ne=0; ne<NEVP; ne++)
  {
    if(draw[ne])
    {
      spinfile[ne]->cd();
      for(no=0; no<NOTRO; no++)
      {
        gr[ne][no] = (TGraphErrors*) spinfile[ne]->Get(gr_n[no].Data());
        gr[ne][no]->SetMarkerColor(evp_plot_color[ne]);
        gr[ne][no]->SetLineColor(evp_plot_color[ne]);
        gr[ne][no]->SetFillColor(evp_plot_color[ne]);
        gr[ne][no]->SetMarkerStyle(3003);
        gr[ne][no]->SetMarkerSize(1.3);
        gr[ne][no]->SetMarkerStyle(kFullCircle);
        gr[ne][no]->SetLineWidth(2);
        multi_gr[no]->Add(gr[ne][no]);
      };
    };
  };


  // build legend
  TLegend * leg;
  leg = new TLegend(0.1,0.9,0.2,0.6);
  for(ne=0; ne<NEVP; ne++) 
  {
    if(draw[ne])
    {
      leg->AddEntry(gr[ne][0],evp_name[ne].Data(),"LPE");
    };
  };


  // draw canvases
  TCanvas * canv[NOTRO];
  TString canv_n[NOTRO];
  for(no=0; no<NOTRO; no++)
  {
    canv_n[no] = Form("canv_%d",no);
    canv[no] = new TCanvas(canv_n[no].Data(),canv_n[no].Data(),1000,1000);
    canv[no]->SetGrid(1,1);
    multi_gr[no]->Draw("APE");
    leg->Draw();
  };
};
