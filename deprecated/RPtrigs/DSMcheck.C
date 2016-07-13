// compares bits at different DSM layers
//
  // comparison selection via argument "compare"
  // 0 - QT to static TCU (static TCU assumes TCU never changed)
  // 1 - QT to dynamic TCU (dynamic TCU accounts for possible changes (my class) )
  // 2 - static TCU to dynamic TCU

void DSMcheck(Int_t compare=0, Int_t runnum=16080030,Int_t segnum=50)
{
  // open trigger tree
  TString dayname = Form("Day%03d",runnum/1000%1000);
  TString filename = Form("../../Run15/%s/run%d.%d.root",dayname.Data(),runnum,segnum);
  cout << filename << endl;
  TFile * infile = new TFile(filename.Data(),"READ");
  TTree * tr = (TTree*) infile->Get("h111");
  UInt_t fpde[8]; // PP101 inputs
  UInt_t lastdsm[8]; // TCU inputs (for static reading, i.e., assume no changes during run15)
  tr->SetBranchAddress("Fpde",fpde);
  tr->SetBranchAddress("lastdsm",lastdsm);


  // load spin15t lib for dynamically reading TCU inputs (i.e., from trig ids from database)
  gSystem->Load("src/RunInfo.so");
  TCUbits * tcu = new TCUbits();


  // PP101 input channel
  // PP001 16 "good hit" bits output to channel 0 of PP101; Chris Perkins says
  // these 16 bits are copied to the second PP001 output which is connected to 
  // PP101 input channel 1... this variable is the PP101 input channel to use
  const Int_t PP101_INPUT = 0;


  // DSM input channel is remapped to branch array indices:
  //                  0 1 2 3 4 5 6 7
  Int_t ChToIdx[8] = {3,2,1,0,7,6,5,4};


  // TCU input channel (for static reading.. to be compared with dynamic reader)
  // TF201 bits 0-15 are sent to TCU input channel 0.. these contain RP bits EOR,WOR,ET,IT
  // TF201 bits 15-31 are sent to TCU input channel 2.. no idea what they are
  // I don't think the TCU is remapped in the trigger tree via ChToIdx map above
  const Int_t TCU_INPUT = 0;


  // PP001 output bits --> PP101 channel 0 input bits (possibly copied to PP101 channel 1 input)
  Int_t E1U1bit = 10;
  Int_t E1U2bit = 11;
  Int_t E1D1bit = 8;
  Int_t E1D2bit = 9;

  Int_t E2U1bit = 0;
  Int_t E2U2bit = 1;
  Int_t E2D1bit = 2;
  Int_t E2D2bit = 3;

  Int_t W1U1bit = 14;
  Int_t W1U2bit = 15;
  Int_t W1D1bit = 12;
  Int_t W1D2bit = 13;

  Int_t W2U1bit = 4;
  Int_t W2U2bit = 5;
  Int_t W2D1bit = 6;
  Int_t W2D2bit = 7;


  // TF201 output bits --> TCU channel 0 input bits
  Int_t EORbit = 13;
  Int_t WORbit = 14;
  Int_t ETbit = 2;
  Int_t ITbit = 12;

  // TF201 scaler bits (possibly sent to TCU input channel 2)
  Int_t EUbit = 3;
  Int_t EDbit = 4;
  Int_t WUbit = 5;
  Int_t WDbit = 6;


  // booleans for software DSM algorithm
  Bool_t E1U1,E1U2,E1D1,E1D2,E2U1,E2U2,E2D1,E2D2,W1U1,W1U2,W1D1,W1D2,W2U1,W2U2,W2D1,W2D2; // level 0
  Bool_t E1U,E1D,E2U,E2D,W1U,W1D,W2U,W2D,EU,ED,WU,WD,EA,EB,IU,ID; // levels 1 & 2

  
  // final bits to be compared
  Bool_t EOR_QT,WOR_QT,ET_QT,IT_QT; // from PP001 through "software" DSM algorithm, implmented below
  Bool_t EOR_TCUs,WOR_TCUs,ET_TCUs,IT_TCUs; // TCU input bits (using static input addresses)
  Bool_t EOR_TCUd,WOR_TCUd,ET_TCUd,IT_TCUd; // TCU input bits (using dynamic input addresses)

  // match counts
  Bool_t EOR_match,WOR_match,ET_match,IT_match;
  Int_t EOR_matchcnt,WOR_matchcnt,ET_matchcnt,IT_matchcnt;
  Float_t EOR_matchperc,WOR_matchperc,ET_matchperc,IT_matchperc;
  Int_t nomatch=0;
  EOR_matchcnt=WOR_matchcnt=ET_matchcnt=IT_matchcnt=0;


  // bit streams
  UInt_t L0STREAM;
  UInt_t L2STREAM;


  
  // tree loop
  for(int i=0; i<tr->GetEntries(); i++)
  {
    tr->GetEntry(i);

    // read data streams
    L0STREAM = fpde[ ChToIdx[PP101_INPUT] ];
    L2STREAM = lastdsm[TCU_INPUT];


    // PP001 output bits
    E1U1 = ( L0STREAM >> E1U1bit) & 0x1;
    E1U2 = ( L0STREAM >> E1U2bit) & 0x1;
    E1D1 = ( L0STREAM >> E1D1bit) & 0x1;
    E1D2 = ( L0STREAM >> E1D2bit) & 0x1;

    E2U1 = ( L0STREAM >> E2U1bit) & 0x1;
    E2U2 = ( L0STREAM >> E2U2bit) & 0x1;
    E2D1 = ( L0STREAM >> E2D1bit) & 0x1;
    E2D2 = ( L0STREAM >> E2D2bit) & 0x1;

    W1U1 = ( L0STREAM >> W1U1bit) & 0x1;
    W1U2 = ( L0STREAM >> W1U2bit) & 0x1;
    W1D1 = ( L0STREAM >> W1D1bit) & 0x1;
    W1D2 = ( L0STREAM >> W1D2bit) & 0x1;

    W2U1 = ( L0STREAM >> W2U1bit) & 0x1;
    W2U2 = ( L0STREAM >> W2U2bit) & 0x1;
    W2D1 = ( L0STREAM >> W2D1bit) & 0x1;
    W2D2 = ( L0STREAM >> W2D2bit) & 0x1;


    // PP101 DSM algorithm implementation
    E1U = E1U1 || E1U2;
    E1D = E1D1 || E1D2;
    E2U = E2U1 || E2U2;
    E2D = E2D1 || E2D2;

    W1U = W1U1 || W1U2;
    W1D = W1D1 || W1D2;
    W2U = W2U1 || W2U2;
    W2D = W2D1 || W2D2;

    EU = E1U || E2U;
    ED = E1D || E2D;

    WU = W1U || W2U;
    WD = W1D || W2D;

    EA = EU && WD;
    EB = ED && WU;
    IU = EU && WU;
    ID = ED && WD;

    ET_QT = EA || EB;
    IT_QT = IU || ID;
    EOR_QT = EU || ED;
    WOR_QT = WU || WD;


    // static TCU input bits
    EOR_TCUs = ( L2STREAM >> EORbit ) & 0x1;
    WOR_TCUs = ( L2STREAM >> WORbit ) & 0x1;
    ET_TCUs = ( L2STREAM >> ETbit ) & 0x1;
    IT_TCUs = ( L2STREAM >> ITbit ) & 0x1;


    // dynamic TCU input bits (from TCUbits class)
    for(int x=0; x<8; x++) tcu->lastdsm[x] = lastdsm[x];
    EOR_TCUd = tcu->Fired("RP_EOR");
    WOR_TCUd = tcu->Fired("RP_WOR");
    ET_TCUd = tcu->Fired("RP_ET");
    IT_TCUd = tcu->Fired("RP_IT");



    if(compare==0)
    {
      EOR_match = (EOR_QT == EOR_TCUs);
      WOR_match = (WOR_QT == WOR_TCUs);
      ET_match = (ET_QT == ET_TCUs);
      IT_match = (IT_QT == IT_TCUs);
    }
    else if(compare==1)
    {
      EOR_match = (EOR_QT == EOR_TCUd);
      WOR_match = (WOR_QT == WOR_TCUd);
      ET_match = (ET_QT == ET_TCUd);
      IT_match = (IT_QT == IT_TCUd);
    }
    else if(compare==2)
    {
      EOR_match = (EOR_TCUs == EOR_TCUd);
      WOR_match = (WOR_TCUs == WOR_TCUd);
      ET_match = (ET_TCUs == ET_TCUd);
      IT_match = (IT_TCUs == IT_TCUd);
    };


    if(EOR_match) EOR_matchcnt++;
    if(WOR_match) WOR_matchcnt++;
    if(ET_match) ET_matchcnt++;
    if(IT_match) IT_matchcnt++;

    
    // print only if there's a mismatch
    if(!EOR_match || !WOR_match || !ET_match || !IT_match)
    {
      printf("%d %d %d %d | %d %d %d %d | %d %d %d %d (ev%d)\n",
        EOR_QT,   WOR_QT,   ET_QT,   IT_QT,
        EOR_TCUs, WOR_TCUs, ET_TCUs, IT_TCUs,
        EOR_match,WOR_match,ET_match,IT_match,
        i);
      nomatch++;
    };
  };
  printf("---------------------------\n");
  printf("%d/%d mismatch\n",nomatch,tr->GetEntries());
  EOR_matchperc = ((Float_t)EOR_matchcnt)/(tr->GetEntries());
  WOR_matchperc = ((Float_t)WOR_matchcnt)/(tr->GetEntries());
  ET_matchperc = ((Float_t)ET_matchcnt)/(tr->GetEntries());
  IT_matchperc = ((Float_t)IT_matchcnt)/(tr->GetEntries());

  printf("EOR match: %.2f%%\n",EOR_matchperc*100);
  printf("WOR match: %.2f%%\n",WOR_matchperc*100);
  printf("ET match: %.2f%%\n",ET_matchperc*100);
  printf("IT match: %.2f%%\n",IT_matchperc*100);

  gSystem->RedirectOutput("match.log");
  printf("%d %d %f %f %f %f\n",runnum,segnum,EOR_matchperc,WOR_matchperc,ET_matchperc,IT_matchperc);
  gSystem->RedirectOutput(0);
};
