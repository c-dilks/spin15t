// draws plots from diag.root

void DrawDiagnostics(const char * filename="diag.root")
{
  TFile * infile = new TFile(filename,"READ");
  Int_t RESX = 400;
  Int_t RESY = 2000;

  gSystem->Load("src/RunData.so");
  Trigger * T = new Trigger();

  gStyle->SetOptStat(0);

  // class names
  const Int_t N_CLASS=3;
  enum class_enum {kSph,kPi0,kThr};
  char class_name[N_CLASS][16];
  strcpy(class_name[kSph],"sph");
  strcpy(class_name[kPi0],"pi0");
  strcpy(class_name[kThr],"thr");

  // trigger names
  Int_t N_TRIG_tmp = T->N;
  const Int_t N_TRIG = N_TRIG_tmp;

  // initialise tcanvases
  TCanvas * trig_canv = new TCanvas("trig_canv","trig_canv",500,500);
  TCanvas * corr_canv[N_CLASS];
  char corr_canv_n[N_CLASS][16];
  for(Int_t c=0; c<N_CLASS; c++)
  {
    sprintf(corr_canv_n[c],"corr_canv_%s",class_name[c]);
    corr_canv[c] = new TCanvas(corr_canv_n[c],corr_canv_n[c],RESX,RESY);
    corr_canv[c]->Divide(1,6);
    for(Int_t cc=1; cc<=6; cc++) corr_canv[c]->GetPad(cc)->SetLogz();
  };
  TCanvas * pi0_canv = new TCanvas("pi0_canv","pi0_canv",RESX,RESY);
  pi0_canv->Divide(1,6);
  for(Int_t cc=1; cc<=4; cc++) pi0_canv->GetPad(cc)->SetLogz();


  // read object arrays
  TObjArray * pt_vs_eta_arr[N_CLASS];
  TObjArray * en_vs_eta_arr[N_CLASS];
  TObjArray * pt_vs_phi_arr[N_CLASS];
  TObjArray * en_vs_phi_arr[N_CLASS];
  TObjArray * eta_vs_phi_arr[N_CLASS];
  TObjArray * pt_vs_en_arr[N_CLASS];
  TObjArray * pi0_z_vs_eta_arr = (TObjArray*) infile->Get("pi0_z_vs_eta_arr");
  TObjArray * pi0_z_vs_phi_arr = (TObjArray*) infile->Get("pi0_z_vs_phi_arr");
  TObjArray * pi0_mass_vs_en_arr = (TObjArray*) infile->Get("pi0_mass_vs_en_arr");
  TObjArray * pi0_mass_vs_pt_arr = (TObjArray*) infile->Get("pi0_mass_vs_pt_arr");
  TObjArray * pi0_mass_dist_arr = (TObjArray*) infile->Get("pi0_mass_dist_arr");
  TObjArray * pi0_z_dist_arr = (TObjArray*) infile->Get("pi0_z_dist_arr");
  char pt_vs_eta_arr_n[N_CLASS][32];
  char en_vs_eta_arr_n[N_CLASS][32];
  char pt_vs_phi_arr_n[N_CLASS][32];
  char en_vs_phi_arr_n[N_CLASS][32];
  char eta_vs_phi_arr_n[N_CLASS][32];
  char pt_vs_en_arr_n[N_CLASS][32];
  for(Int_t c=0; c<N_CLASS; c++)
  {
    sprintf(pt_vs_eta_arr_n[c],"%s_pt_vs_eta_arr",class_name[c]);
    sprintf(en_vs_eta_arr_n[c],"%s_en_vs_eta_arr",class_name[c]);
    sprintf(pt_vs_phi_arr_n[c],"%s_pt_vs_phi_arr",class_name[c]);
    sprintf(en_vs_phi_arr_n[c],"%s_en_vs_phi_arr",class_name[c]);
    sprintf(eta_vs_phi_arr_n[c],"%s_eta_vs_phi_arr",class_name[c]);
    sprintf(pt_vs_en_arr_n[c],"%s_pt_vs_en_arr",class_name[c]);
    pt_vs_eta_arr[c] = (TObjArray*) infile->Get(pt_vs_eta_arr_n[c]);
    en_vs_eta_arr[c] = (TObjArray*) infile->Get(en_vs_eta_arr_n[c]);
    pt_vs_phi_arr[c] = (TObjArray*) infile->Get(pt_vs_phi_arr_n[c]);
    en_vs_phi_arr[c] = (TObjArray*) infile->Get(en_vs_phi_arr_n[c]);
    eta_vs_phi_arr[c] = (TObjArray*) infile->Get(eta_vs_phi_arr_n[c]);
    pt_vs_en_arr[c] = (TObjArray*) infile->Get(pt_vs_en_arr_n[c]);
  };

  TH1D * trig_dist = (TH1D*) infile->Get("trig_dist");

  char diag_dir[16]; strcpy(diag_dir,"diag_plots");
  char trig_canv_png[64]; sprintf(trig_canv_png,"%s/trig.png",diag_dir);

  // pdf names -- produces ~12 MB files... too slow!
  /*
  char corr_canv_pdf[N_CLASS][64];
  char pi0_canv_pdf[64];
  sprintf(pi0_canv_pdf,"%s/pi0.pdf",diag_dir);
  for(Int_t c=0; c<N_CLASS; c++) sprintf(corr_canv_pdf[c],"%s/%s_corr.pdf",diag_dir,class_name[c]);
  char corr_canv_pdfL[N_CLASS][64];
  char pi0_canv_pdfL[64];
  char corr_canv_pdfR[N_CLASS][64];
  char pi0_canv_pdfR[64];
  sprintf(pi0_canv_pdfL,"%s(",pi0_canv_pdf);
  for(Int_t c=0; c<N_CLASS; c++) sprintf(corr_canv_pdfL[c],"%s(",corr_canv_pdf[c]);
  sprintf(pi0_canv_pdfR,"%s)",pi0_canv_pdf);
  for(Int_t c=0; c<N_CLASS; c++) sprintf(corr_canv_pdfR[c],"%s)",corr_canv_pdf[c]);
  */

  // png names
  char corr_canv_png[N_CLASS][N_TRIG][64];
  char pi0_canv_png[N_TRIG][64];
  for(Int_t t=0; t<N_TRIG; t++)
  {
    sprintf(pi0_canv_png[t],"%s/pi0_corr_MZ/pi0_%s.png",diag_dir,T->Name(t));
    for(Int_t c=0; c<N_CLASS; c++) sprintf(corr_canv_png[c][t],"%s/%s_corr/%s_corr_%s.png",
       diag_dir,class_name[c],class_name[c],T->Name(t));
  };


  // print pdfs & pngs
  trig_canv->cd();
  trig_dist->Draw();
  trig_canv->Print(trig_canv_png,"png");
  for(Int_t t=0; t<N_TRIG; t++)
  {
    for(Int_t c=0; c<N_CLASS; c++)
    {
      corr_canv[c]->cd(1); pt_vs_eta_arr[c]->At(t)->Draw("colz");
      corr_canv[c]->cd(2); pt_vs_phi_arr[c]->At(t)->Draw("colz");
      corr_canv[c]->cd(3); en_vs_eta_arr[c]->At(t)->Draw("colz");
      corr_canv[c]->cd(4); en_vs_phi_arr[c]->At(t)->Draw("colz");
      corr_canv[c]->cd(5); eta_vs_phi_arr[c]->At(t)->Draw("colz");
      corr_canv[c]->cd(6); pt_vs_en_arr[c]->At(t)->Draw("colz");
      //if(t==0) corr_canv[c]->Print(corr_canv_pdfL[c],"pdf");
      //else if(t+1==N_TRIG) corr_canv[c]->Print(corr_canv_pdfR[c],"pdf");
      //else corr_canv[c]->Print(corr_canv_pdf[c],"pdf");
      corr_canv[c]->Print(corr_canv_png[c][t],"png");
    };
    pi0_canv->cd(1); pi0_z_vs_eta_arr->At(t)->Draw("colz");
    pi0_canv->cd(2); pi0_z_vs_phi_arr->At(t)->Draw("colz");
    pi0_canv->cd(3); pi0_mass_vs_en_arr->At(t)->Draw("colz");
    pi0_canv->cd(4); pi0_mass_vs_pt_arr->At(t)->Draw("colz");
    pi0_canv->cd(5); pi0_mass_dist_arr->At(t)->Draw();
    pi0_canv->cd(6); pi0_z_dist_arr->At(t)->Draw();
    //if(t==0) pi0_canv->Print(pi0_canv_pdfL,"pdf");
    //else if(t+1==N_TRIG) pi0_canv->Print(pi0_canv_pdfR,"pdf");
    //else pi0_canv->Print(pi0_canv_pdf,"pdf");
    pi0_canv->Print(pi0_canv_png[t],"png");
  };


  // build html page
  gSystem->RedirectOutput("diag_web.html","w");
  printf("<html>\n<head><title>Kinematic Correlations</title></head>\n<body>\n");
  printf("<img src=%s>\n",trig_canv_png);
  printf("<div style=\"width:%dpx;overflow:scroll;\">\n",RESX*(N_TRIG+1));
  for(Int_t c=0; c<N_CLASS; c++)
  {
    printf("<hr />\n");
    for(Int_t t=0; t<N_TRIG; t++) printf("<img src=%s>\n",corr_canv_png[c][t]);
    if(c==kPi0) for(Int_t t=0; t<N_TRIG; t++) printf("<img src=%s>\n",pi0_canv_png[t]);
  };
  printf("</div>\n</body>\n</html>\n");
  gSystem->RedirectOutput(0);
  system("tar czvf diag_htmlfiles.tar.gz diag_web.html diag_plots");
};
