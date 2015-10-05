void mk_tree()
{
  const Int_t NTRIG = 11;
  TString trg_name[NTRIG] = {"RP_SD","RP_SDT","RP_RPZMU","RP_RPZE","RP_ET","RP_CP","RP_RP2MU","RP_RP2E","RP_Zerobias","RP_CPT2","RP_CPEI"};
  TString trg_name_ps[NTRIG];
  Int_t i;
  for(i=0; i<NTRIG; i++) 
  {
    trg_name_ps[i] = trg_name[i]+"_ps/D";
    trg_name[i] = trg_name[i]+"/D";
  };
  TString branches = "i/I:runnum/I:day/I:run/I:type/C:fill/F:E/F:t/D:freq/D";
  for(i=0; i<NTRIG; i++) branches = branches+":"+trg_name[i]+":"+trg_name_ps[i];
  cout << branches << endl;
  TFile * of = new TFile("rptree.root","RECREATE");
  TTree * tr = new TTree();
  tr->ReadFile("trigger_culled.dat",branches.Data());
  tr->Write("tr");
  tr->Print();
};

