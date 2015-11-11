{
  gStyle->SetOptStat(0);
  gSystem->Load("src/RunInfo.so");
  BBCtiles * bbc = new BBCtiles();
  bbc->DrawBBC();
  bbc->PrintBBC();
};
