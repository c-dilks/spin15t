void ENT(const char * filename="diag_EOR.set081s3.root", Bool_t output=1)
{
  /////
  TString plot = "mass_dist_for_ptbin_3_arr";
  Int_t dim = 1; 
  Int_t num = 7;
  /////
  

  TFile * infile = new TFile(filename,"READ");
  if(output) gSystem->RedirectOutput("counts.txt","a");
  switch(dim)
  {
    case 1:
      printf("%ld\n",((TH1D*)((TObjArray*)(infile->Get(plot.Data())))->At(num))->GetEntries());
      break;
    case 2:
      printf("%ld\n",((TH2D*)((TObjArray*)(infile->Get(plot.Data())))->At(num))->GetEntries());
      break;
  };
  if(output) gSystem->RedirectOutput(0);
};
