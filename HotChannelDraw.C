// reads chandists/*.root and makes a pdf file
// of the adc-weighted channel distributions

void HotChannelDraw(const char * dir="chandists/")
{
  // obtain list of files in chandists
  TSystemDirectory td(dir,dir);
  TList * files = td.GetListOfFiles();
  files->Sort();

  const Int_t NMAX=10;

  Int_t NF_tmp = files->GetEntries();
  const Int_t NF=NF_tmp;
  TString filename;
  TFile * infile[NF];
  Int_t runnum[NF];
  TH1D * dist[4][NF];
  TH1D * max[NMAX][4];
  TString max_n,max_t;
  Int_t nchan;
  for(Int_t n=0; n<4; n++)
  {
    for(Int_t m=0; m<NMAX; m++)
    {
      if(m==0) max_t.Form("Max channel for n%d",n+1);
      else if(m==1) max_t.Form("2nd max channel for n%d",n+1);
      else if(m==2) max_t.Form("3rd max channel for n%d",n+1);
      else max_t.Form("%dth max channel for n%d",m+1,n+1);
      max_n.Form("max%dn%d",m+1,n+1);
      nchan = (34-10*(n>1)) * (17-5*(n>1));
      max[m][n] = new TH1D(max_n.Data(),max_t.Data(),nchan,1,nchan+1);
    };
  };

  TSystemFile * file;
  Int_t c=0;
  TIter nxt(files);
  nxt(); // skip dir/.
  nxt(); // skip dir/..
  while(file = (TSystemFile*)nxt())
  {
    filename = TString(dir) + file->GetName();
    sscanf(filename(filename.Length()-13,8).Data(),"%d",&(runnum[c]));
    //cout << c << " " << filename << " " << runnum[c] << endl;
    infile[c] = new TFile(filename.Data(),"READ");
    dist[0][c] = (TH1D*) infile[c]->Get("chandist1");
    dist[1][c] = (TH1D*) infile[c]->Get("chandist2");
    dist[2][c] = (TH1D*) infile[c]->Get("chandist3");
    dist[3][c] = (TH1D*) infile[c]->Get("chandist4");
    c++;
  };
  cout << c << " files" << endl;


  // print chandists and get first maximum
  TCanvas * canv = new TCanvas("canv","canv",1400,700);
  gStyle->SetOptStat(0);
  TString disttitle;
  TString pdffile = "hot_chan.pdf";
  TString pdffileL = pdffile+"(";
  TString pdffileR = pdffile+")";
  for(Int_t i=0; i<c; i++)
  {
    canv->Clear();
    canv->Divide(2,2);
    for(Int_t n=0; n<4; n++) 
    {
      canv->GetPad(n+1)->SetGrid(1,1);
      canv->cd(n+1);
      disttitle.Form("n%d r%d",n+1,runnum[i]);
      dist[n][i]->SetTitle(disttitle.Data());
      dist[n][i]->SetFillColor(kGreen+2);
      dist[n][i]->SetLineColor(kGreen+2);
      dist[n][i]->Draw();
      max[0][n]->Fill(dist[n][i]->GetMaximumBin());
    };
    if(i==0) canv->Print(pdffileL.Data(),"pdf");
    else canv->Print(pdffile.Data(),"pdf");
    cout << runnum[i] << endl;
  };

  // compute second and third maxima
  Int_t q;
  for(Int_t i=0; i<c; i++)
  {
    for(Int_t n=0; n<4; n++) 
    {
      for(Int_t m=1; m<NMAX; m++)
      {
        q = dist[n][i]->GetMaximumBin();
        dist[n][i]->SetBinContent(q,0);
        max[m][n]->Fill(dist[n][i]->GetMaximumBin());
      };
    };
  };


  for(Int_t m=0; m<NMAX; m++)
  {
    canv->Clear();
    canv->Divide(2,2);
    for(Int_t n=0; n<4; n++) 
    {
      canv->GetPad(n+1)->SetGrid(1,1);
      canv->cd(n+1);
      max[m][n]->SetLineColor(kRed);
      max[m][n]->SetFillColor(kRed);
      max[m][n]->Draw();
    };
    canv->Print(pdffile.Data(),"pdf");
  };
  canv->Clear();
  canv->Print(pdffileR.Data(),"pdf");

  TFile * outfile = new TFile("hot_chan.root","RECREATE");
  for(Int_t m=0; m<NMAX; m++)
  {
    for(Int_t n=0; n<4; n++) max[m][n]->Write();
  };
};
