void EVPdiagnostics(TString setname="079ai")
{
  int c,ew,sl;
  enum ew_enum {kE,kW};

  TString filename = "evpset/diag_N.set"+setname+".root";
  TFile * infile = new TFile(filename.Data(),"READ");
  TTree * tr = (TTree*) infile->Get("tr");

  const Int_t NMULTBINS=16;

  TH1D * Xflow_dist[NMULTBINS][2]; // [multiplicity bin] [ew]
  TH1D * Yflow_dist[NMULTBINS][2]; // [multiplicity bin] [ew]
  TString Xflow_n[NMULTBINS][2];
  TString Yflow_n[NMULTBINS][2];
  TString ew_str[2];
  ew_str[kE]="e";
  ew_str[kW]="w";

  /*
  Int_t lbound[NMULTBINS];
  Int_t ubound[NMULTBINS];
  for(c=0; c<NMULTBINS; c++)
  {
    lbound[c]=c+1;
    ubound[c]=c+2;
  };
  */

  Double_t bin_lim=6000;
  Int_t nbins=100;

  TString flow_cut[NMULTBINS][2];
  TString Xflow_var[NMULTBINS][2];
  TString Yflow_var[NMULTBINS][2];

  Int_t line_color[NMULTBINS];
  line_color[0] = (Int_t) kRed-2;
  line_color[1] = (Int_t) kRed;
  line_color[2] = (Int_t) kOrange;
  line_color[3] = (Int_t) kYellow+1;
  line_color[4] = (Int_t) kGreen;
  line_color[5] = (Int_t) kGreen-6;
  line_color[6] = (Int_t) kTeal;
  line_color[7] = (Int_t) kTeal-7;
  line_color[8] = (Int_t) kCyan+3;
  line_color[9] = (Int_t) kAzure;
  line_color[10] = (Int_t) kBlue+2;
  line_color[11] = (Int_t) kViolet+2;
  line_color[12] = (Int_t) kViolet;
  line_color[13] = (Int_t) kMagenta-9;
  line_color[14] = (Int_t) kGray;
  line_color[15] = (Int_t) kBlack;

  for(ew=0; ew<2; ew++)
  {
    for(c=0; c<NMULTBINS; c++)
    {
      Xflow_n[c][ew] = Form("Xflow_%s_QTN_%d",ew_str[ew].Data(),c+1);
      Yflow_n[c][ew] = Form("Yflow_%s_QTN_%d",ew_str[ew].Data(),c+1);

      Xflow_dist[c][ew] = new TH1D(Xflow_n[c][ew].Data(),Xflow_n[c][ew],nbins,-1*bin_lim,bin_lim);
      Yflow_dist[c][ew] = new TH1D(Yflow_n[c][ew].Data(),Yflow_n[c][ew],nbins,-1*bin_lim,bin_lim);

      Xflow_dist[c][ew]->SetLineWidth(2);
      Yflow_dist[c][ew]->SetLineWidth(2);
      Xflow_dist[c][ew]->SetLineColor(line_color[c]);
      Yflow_dist[c][ew]->SetLineColor(line_color[c]);

      flow_cut[c][ew] = Form("evp_%ss<1000 && qtn%s==%d && abs(vertex)<200",ew_str[ew].Data(),ew_str[ew].Data(),c+1);
      Xflow_var[c][ew] = Form("Xflow_%ss",ew_str[ew].Data());
      Yflow_var[c][ew] = Form("Yflow_%ss",ew_str[ew].Data());
      printf("c=%d ew=%d flowcut=\"%s\"\n",c,ew,flow_cut[c][ew].Data());

      tr->Project(Xflow_n[c][ew].Data(),Xflow_var[c][ew].Data(),flow_cut[c][ew]);
      tr->Project(Yflow_n[c][ew].Data(),Yflow_var[c][ew].Data(),flow_cut[c][ew]);
    };
  };
  

  TCanvas * canv_flow = new TCanvas("canv_flow","canv_flow",1000,500);
  canv_flow->Divide(2,2);
  for(int cc=1; cc<=4; cc++)
  {
    canv_flow->cd(cc);
    for(c=0; c<NMULTBINS; c++)
    {
      if(cc<3)
      {
        if(c==0) Xflow_dist[c][(cc+1)%2]->Draw();
        else Xflow_dist[c][(cc+1)%2]->Draw("same");
      }
      else
      {
        if(c==0) Yflow_dist[c][(cc+1)%2]->Draw();
        else Yflow_dist[c][(cc+1)%2]->Draw("same");
      }
    };
  };

  // flow mean with RMS as error bars
  TGraphErrors * Xflow_mean[2];
  TGraphErrors * Yflow_mean[2];
  TString Xflow_mean_n[2];
  TString Yflow_mean_n[2];

  for(ew=0; ew<2; ew++)
  {
    Xflow_mean_n[ew] = Form("%s Xflow mean vs. QTN%s",ew_str[ew].Data(),ew_str[ew].Data());
    Yflow_mean_n[ew] = Form("%s Yflow mean vs. QTN%s",ew_str[ew].Data(),ew_str[ew].Data());

    Xflow_mean[ew] = new TGraphErrors();
    Yflow_mean[ew] = new TGraphErrors();

    Xflow_mean[ew]->SetName(Xflow_mean_n[ew].Data());
    Xflow_mean[ew]->SetTitle(Xflow_mean_n[ew].Data());
    Yflow_mean[ew]->SetName(Yflow_mean_n[ew].Data());
    Yflow_mean[ew]->SetTitle(Yflow_mean_n[ew].Data());

    Xflow_mean[ew]->SetMarkerColor(kRed);
    Xflow_mean[ew]->SetMarkerStyle(kFullCircle);
    Xflow_mean[ew]->SetMarkerSize(2);
    Yflow_mean[ew]->SetMarkerColor(kRed);
    Yflow_mean[ew]->SetMarkerStyle(kFullCircle);
    Yflow_mean[ew]->SetMarkerSize(2);

    for(c=0; c<NMULTBINS; c++)
    {
      Xflow_mean[ew]->SetPoint(c,c+1,Xflow_dist[c][ew]->GetMean());
      Xflow_mean[ew]->SetPointError(c,0,Xflow_dist[c][ew]->GetRMS());

      Yflow_mean[ew]->SetPoint(c,c+1,Yflow_dist[c][ew]->GetMean());
      Yflow_mean[ew]->SetPointError(c,0,Yflow_dist[c][ew]->GetRMS());
    };
  };


  // print centering calibration code
  Double_t mmx,mmy,rrx,rry;
  for(ew=0; ew<2; ew++)
  {
    for(sl=0; sl<2; sl++)
    {
      for(c=0; c<NMULTBINS; c++)
      {
        if(sl==1) { mmx=0; mmy=0; rrx=1; rry=1; }
        else
        {
          mmx = Xflow_dist[c][ew]->GetMean();
          rrx = Xflow_dist[c][ew]->GetRMS();

          mmy = Yflow_dist[c][ew]->GetMean();
          rry = Yflow_dist[c][ew]->GetRMS();
        };

        printf("  Xflow_calib[%d][%d][%d][0]=%.2f; Xflow_calib[%d][%d][%d][1]=%.2f;\n",ew,sl,c,mmx,ew,sl,c,rrx);
        printf("  Yflow_calib[%d][%d][%d][0]=%.2f; Yflow_calib[%d][%d][%d][1]=%.2f;\n",ew,sl,c,mmy,ew,sl,c,rry);
      };
    };
  };

  // draw flow mean graphs
  TCanvas * flow_mean_canv = new TCanvas("flow_mean_canv","flow_mean_canv",1000,1000);
  flow_mean_canv->Divide(2,2);
  flow_mean_canv->cd(1); Xflow_mean[kE]->Draw("APE");
  flow_mean_canv->cd(2); Xflow_mean[kW]->Draw("APE");
  flow_mean_canv->cd(3); Yflow_mean[kE]->Draw("APE");
  flow_mean_canv->cd(4); Yflow_mean[kW]->Draw("APE");
};
