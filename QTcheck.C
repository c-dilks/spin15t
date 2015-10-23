// checks OFile RP QT branches against trigger bits, effectively checking output of PullRP

void QTcheck(Int_t compare=0, const char * setname="085s5")
{
  enum ew_enum {kE,kW};
  enum io_enum {kI,kO};
  enum ud_enum {kU,kD};
  enum ns_enum {kN,kS};

  // open OFile
  TString filename = Form("../../OFiles/OFset%s.root",setname);
  cout << filename << endl;
  TFile * infile = new TFile(filename.Data(),"READ");
  TTree * tr = (TTree*) infile->Get("p_out");
  UInt_t fpde[8]; // PP101 inputs
  UInt_t lastdsm[8]; // TCU inputs (for static reading, i.e., assume no changes during run15)
  Char_t QTNE,QTNW;
  Char_t IdxE[8];
  Char_t IdxW[8];
  Short_t ADCE[8];
  Short_t ADCW[8];
  Short_t TACE[8];
  Short_t TACW[8];
  Int_t runnum;
  tr->SetBranchAddress("br_Rnum",&runnum);
  tr->SetBranchAddress("br_Fpde",fpde);
  tr->SetBranchAddress("br_lastdsm",lastdsm);
  tr->SetBranchAddress("br_RPE_QTN",&QTNE);
  tr->SetBranchAddress("br_RPW_QTN",&QTNW);
  tr->SetBranchAddress("br_RPE_Idx",IdxE);
  tr->SetBranchAddress("br_RPE_ADC",ADCE);
  tr->SetBranchAddress("br_RPE_TAC",TACE);
  tr->SetBranchAddress("br_RPW_Idx",IdxW);
  tr->SetBranchAddress("br_RPW_ADC",ADCW);
  tr->SetBranchAddress("br_RPW_TAC",TACW);



  // load spin15t lib for dynamically reading TCU inputs (i.e., from trig ids from database)
  gSystem->Load("src/RunInfo.so");
  TCUbits * tcu = new TCUbits();
  RPscint * rpsci = new RPscint();


  // PP101 input channel
  // PP001 16 "good hit" bits output to channel 0 of PP101; Chris Perkins says
  // these 16 bits are copied to the second PP001 output which is connected to 
  // PP101 input channel 1... this variable is the PP101 input channel to use
  const Int_t PP101_INPUT = 0;
  //const Int_t PP101_INPUT = 1;


  // DSM input channel is remapped to branch array indices:
  //                  0 1 2 3 4 5 6 7
  Int_t ChToIdx[8] = {3,2,1,0,7,6,5,4};


  // TCU input channel (for static reading.. to be compared with dynamic reader)
  // TF201 bits 0-15 are sent to TCU input channel 0.. these contain RP bits EOR,WOR,ET,IT
  // TF201 bits 15-31 are sent to TCU input channel 2.. no idea what they are
  // I don't think the TCU is remapped in the trigger tree via ChToIdx map above
  const Int_t TCU_INPUT = 0;


  // PP001 QT input channel indices
  Int_t E1U1qtchan = rpsci->EiunToIdx(kE,kI,kU,kN);
  Int_t E1U2qtchan = rpsci->EiunToIdx(kE,kI,kU,kS);
  Int_t E1D1qtchan = rpsci->EiunToIdx(kE,kI,kD,kS);
  Int_t E1D2qtchan = rpsci->EiunToIdx(kE,kI,kD,kN);

  Int_t E2U1qtchan = rpsci->EiunToIdx(kE,kO,kU,kN);
  Int_t E2U2qtchan = rpsci->EiunToIdx(kE,kO,kU,kS);
  Int_t E2D1qtchan = rpsci->EiunToIdx(kE,kO,kD,kS);
  Int_t E2D2qtchan = rpsci->EiunToIdx(kE,kO,kD,kN);

  Int_t W1U1qtchan = rpsci->EiunToIdx(kW,kI,kU,kS);
  Int_t W1U2qtchan = rpsci->EiunToIdx(kW,kI,kU,kN);
  Int_t W1D1qtchan = rpsci->EiunToIdx(kW,kI,kD,kN);
  Int_t W1D2qtchan = rpsci->EiunToIdx(kW,kI,kD,kS);

  Int_t W2U1qtchan = rpsci->EiunToIdx(kW,kO,kU,kS);
  Int_t W2U2qtchan = rpsci->EiunToIdx(kW,kO,kU,kN);
  Int_t W2D1qtchan = rpsci->EiunToIdx(kW,kO,kD,kN);
  Int_t W2D2qtchan = rpsci->EiunToIdx(kW,kO,kD,kS);


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


  // PP001 QT input bits
  Bool_t E1U1_QT;
  Bool_t E1U2_QT;
  Bool_t E1D1_QT;
  Bool_t E1D2_QT;
  Bool_t E2U1_QT;
  Bool_t E2U2_QT;
  Bool_t E2D1_QT;
  Bool_t E2D2_QT;
  Bool_t W1U1_QT;
  Bool_t W1U2_QT;
  Bool_t W1D1_QT;
  Bool_t W1D2_QT;
  Bool_t W2U1_QT;
  Bool_t W2U2_QT;
  Bool_t W2D1_QT;
  Bool_t W2D2_QT;

  // booleans for software DSM algorithm
  Bool_t E1U1,E1U2,E1D1,E1D2,E2U1,E2U2,E2D1,E2D2,W1U1,W1U2,W1D1,W1D2,W2U1,W2U2,W2D1,W2D2; // level 0
  Bool_t E1U,E1D,E2U,E2D,W1U,W1D,W2U,W2D,EU,ED,WU,WD,EA,EB,IU,ID; // levels 1 & 2


  // booleans from reading QT
  Bool_t EOR_QT,WOR_QT;

  
  // final bits to be compared
  Bool_t EOR_L1,WOR_L1,ET_L1,IT_L1; // from PP001 through "software" DSM algorithm, implmented below
  Bool_t EOR_TCUs,WOR_TCUs,ET_TCUs,IT_TCUs; // TCU input bits (using static input addresses)
  Bool_t EOR_TCUd,WOR_TCUd,ET_TCUd,IT_TCUd; // TCU input bits (using dynamic input addresses)

  // match counts
  Bool_t EOR_match,WOR_match,ET_match,IT_match;
  Int_t EOR_matchcnt,WOR_matchcnt,ET_matchcnt,IT_matchcnt;
  Float_t EOR_matchperc,WOR_matchperc,ET_matchperc,IT_matchperc;
  Int_t nomatch=0;
  EOR_matchcnt=WOR_matchcnt=ET_matchcnt=IT_matchcnt=0;

  Bool_t E1U1_match;
  Bool_t E1U2_match;
  Bool_t E1D1_match;
  Bool_t E1D2_match;
  Bool_t E2U1_match;
  Bool_t E2U2_match;
  Bool_t E2D1_match;
  Bool_t E2D2_match;
  Bool_t W1U1_match;
  Bool_t W1U2_match;
  Bool_t W1D1_match;
  Bool_t W1D2_match;
  Bool_t W2U1_match;
  Bool_t W2U2_match;
  Bool_t W2D1_match;
  Bool_t W2D2_match;

  Int_t E1U1_matchcnt = 0;
  Int_t E1U2_matchcnt = 0;
  Int_t E1D1_matchcnt = 0;
  Int_t E1D2_matchcnt = 0;
  Int_t E2U1_matchcnt = 0;
  Int_t E2U2_matchcnt = 0;
  Int_t E2D1_matchcnt = 0;
  Int_t E2D2_matchcnt = 0;
  Int_t W1U1_matchcnt = 0;
  Int_t W1U2_matchcnt = 0;
  Int_t W1D1_matchcnt = 0;
  Int_t W1D2_matchcnt = 0;
  Int_t W2U1_matchcnt = 0;
  Int_t W2U2_matchcnt = 0;
  Int_t W2D1_matchcnt = 0;
  Int_t W2D2_matchcnt = 0;

  Float_t E1U1_matchperc;
  Float_t E1U2_matchperc;
  Float_t E1D1_matchperc;
  Float_t E1D2_matchperc;
  Float_t E2U1_matchperc;
  Float_t E2U2_matchperc;
  Float_t E2D1_matchperc;
  Float_t E2D2_matchperc;
  Float_t W1U1_matchperc;
  Float_t W1U2_matchperc;
  Float_t W1D1_matchperc;
  Float_t W1D2_matchperc;
  Float_t W2U1_matchperc;
  Float_t W2U2_matchperc;
  Float_t W2D1_matchperc;
  Float_t W2D2_matchperc;


  // bit streams
  UInt_t L0STREAM;
  UInt_t L2STREAM;


  
  // tree loop
  Int_t ENT = tr->GetEntries();
  ENT=1000;
  for(int i=0; i<ENT; i++)
  {
    tr->GetEntry(i);

    // reset QT input bits
    E1U1_QT = false;
    E1U2_QT = false;
    E1D1_QT = false;
    E1D2_QT = false;
    E2U1_QT = false;
    E2U2_QT = false;
    E2D1_QT = false;
    E2D2_QT = false;
    W1U1_QT = false;
    W1U2_QT = false;
    W1D1_QT = false;
    W1D2_QT = false;
    W2U1_QT = false;
    W2U2_QT = false;
    W2D1_QT = false;
    W2D2_QT = false;

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

    ET_L1 = EA || EB;
    IT_L1 = IU || ID;
    EOR_L1 = EU || ED;
    WOR_L1 = WU || WD;


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


    // QT booleans
    EOR_QT = false;
    WOR_QT = false;
    if(QTNE>0)
    {
      for(int q=0; q<QTNE; q++)
      {
        if(ADCE[q]>100 && TACE[q]>100 && TACE[q]<2500) 
        {
          EOR_QT = true;
          if(IdxE[q] == E1U1qtchan) E1U1_QT = true;
          if(IdxE[q] == E1U2qtchan) E1U2_QT = true;
          if(IdxE[q] == E1D1qtchan) E1D1_QT = true;
          if(IdxE[q] == E1D2qtchan) E1D2_QT = true;
          if(IdxE[q] == E2U1qtchan) E2U1_QT = true;
          if(IdxE[q] == E2U2qtchan) E2U2_QT = true;
          if(IdxE[q] == E2D1qtchan) E2D1_QT = true;
          if(IdxE[q] == E2D2qtchan) E2D2_QT = true;
        };
      };
    };
    if(QTNW>0)
    {
      for(int q=0; q<QTNW; q++)
      {
        if(ADCW[q]>100 && TACW[q]>100 && TACW[q]<2500)
        {
          WOR_QT = true;
          if(IdxW[q] == W1U1qtchan) W1U1_QT = true;
          if(IdxW[q] == W1U2qtchan) W1U2_QT = true;
          if(IdxW[q] == W1D1qtchan) W1D1_QT = true;
          if(IdxW[q] == W1D2qtchan) W1D2_QT = true;
          if(IdxW[q] == W2U1qtchan) W2U1_QT = true;
          if(IdxW[q] == W2U2qtchan) W2U2_QT = true;
          if(IdxW[q] == W2D1qtchan) W2D1_QT = true;
          if(IdxW[q] == W2D2qtchan) W2D2_QT = true;
        };
      };
    };



    if(compare==-1)
    {
      E1U1_match = (E1U1_QT==E1U1);
      E1U2_match = (E1U2_QT==E1U2);
      E1D1_match = (E1D1_QT==E1D1);
      E1D2_match = (E1D2_QT==E1D2);
      E2U1_match = (E2U1_QT==E2U1);
      E2U2_match = (E2U2_QT==E2U2);
      E2D1_match = (E2D1_QT==E2D1);
      E2D2_match = (E2D2_QT==E2D2);
      W1U1_match = (W1U1_QT==W1U1);
      W1U2_match = (W1U2_QT==W1U2);
      W1D1_match = (W1D1_QT==W1D1);
      W1D2_match = (W1D2_QT==W1D2);
      W2U1_match = (W2U1_QT==W2U1);
      W2U2_match = (W2U2_QT==W2U2);
      W2D1_match = (W2D1_QT==W2D1);
      W2D2_match = (W2D2_QT==W2D2);
    }
    else if(compare==0)
    {
      EOR_match = (EOR_QT == EOR_L1);
      WOR_match = (WOR_QT == WOR_L1);
      ET_match = 1 /*(ET_QT == ET_L1)*/;
      IT_match = 1 /*(IT_QT == IT_L1)*/;
    }
    else if(compare==1)
    {
      EOR_match = (EOR_QT == EOR_TCUd);
      WOR_match = (WOR_QT == WOR_TCUd);
      ET_match = 1 /*(ET_QT == ET_TCUd)*/;
      IT_match = 1 /*(IT_QT == IT_TCUd)*/;
    }
    else if(compare==2)
    {
      EOR_match = (EOR_L1 == EOR_TCUd);
      WOR_match = (WOR_L1 == WOR_TCUd);
      ET_match = (ET_L1 == ET_TCUd);
      IT_match = (IT_L1 == IT_TCUd);
    };


    if(EOR_match) EOR_matchcnt++;
    if(WOR_match) WOR_matchcnt++;
    if(ET_match) ET_matchcnt++;
    if(IT_match) IT_matchcnt++;

    if(compare==-1)
    {
      if(E1U1_match) E1U1_matchcnt++;
      if(E1U2_match) E1U2_matchcnt++;
      if(E1D1_match) E1D1_matchcnt++;
      if(E1D2_match) E1D2_matchcnt++;
      if(E2U1_match) E2U1_matchcnt++;
      if(E2U2_match) E2U2_matchcnt++;
      if(E2D1_match) E2D1_matchcnt++;
      if(E2D2_match) E2D2_matchcnt++;
      if(W1U1_match) W1U1_matchcnt++;
      if(W1U2_match) W1U2_matchcnt++;
      if(W1D1_match) W1D1_matchcnt++;
      if(W1D2_match) W1D2_matchcnt++;
      if(W2U1_match) W2U1_matchcnt++;
      if(W2U2_match) W2U2_matchcnt++;
      if(W2D1_match) W2D1_matchcnt++;
      if(W2D2_match) W2D2_matchcnt++;
    };
    
    // print only if there's a mismatch
    if(compare!=-1)
    {
      if(!EOR_match || !WOR_match || !ET_match || !IT_match)
      {
        //printf("%d | %d %d - - | %d %d - - | %d %d - - (ev%d)\n",
          //runnum,
          //EOR_QT,   WOR_QT,   /*ET_QT,   IT_QT,*/
          //EOR_TCUs, WOR_TCUs, /*ET_TCUs, IT_TCUs,*/
          //EOR_match,WOR_match,/*ET_match,IT_match,*/
          //i);
        nomatch++;
      };
    }
    else if(compare==-1)
    {
      if(!E1U1_match ||
         !E1U2_match ||
         !E1D1_match ||
         !E1D2_match ||
         !E2U1_match ||
         !E2U2_match ||
         !E2D1_match ||
         !E2D2_match ||
         !W1U1_match ||
         !W1U2_match ||
         !W1D1_match ||
         !W1D2_match ||
         !W2U1_match ||
         !W2U2_match ||
         !W2D1_match ||
         !W2D2_match)
      {
        printf(" %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d | %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d | %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
         E1U1_QT,E1U2_QT,E1D1_QT,E1D2_QT,E2U1_QT,E2U2_QT,E2D1_QT,E2D2_QT,W1U1_QT,W1U2_QT,W1D1_QT,W1D2_QT,W2U1_QT,W2U2_QT,W2D1_QT,W2D2_QT,
         E1U1,E1U2,E1D1,E1D2,E2U1,E2U2,E2D1,E2D2,W1U1,W1U2,W1D1,W1D2,W2U1,W2U2,W2D1,W2D2,
         E1U1_match,E1U2_match,E1D1_match,E1D2_match,E2U1_match,E2U2_match,E2D1_match,E2D2_match,
         W1U1_match,W1U2_match,W1D1_match,W1D2_match,W2U1_match,W2U2_match,W2D1_match,W2D2_match);
        nomatch++;
      };
    };
  };
  printf("---------------------------\n");


  printf("%d/%d mismatch\n",nomatch,ENT);
  if(compare!=-1)
  {
    EOR_matchperc = ((Float_t)EOR_matchcnt)/(ENT);
    WOR_matchperc = ((Float_t)WOR_matchcnt)/(ENT);
    ET_matchperc = ((Float_t)ET_matchcnt)/(ENT);
    IT_matchperc = ((Float_t)IT_matchcnt)/(ENT);

    printf("EOR match: %.2f%%\n",EOR_matchperc*100);
    printf("WOR match: %.2f%%\n",WOR_matchperc*100);
    /*
    printf("ET match: %.2f%%\n",ET_matchperc*100);
    printf("IT match: %.2f%%\n",IT_matchperc*100);
    */
  }
  else if(compare==-1)
  {
    E1U1_matchperc = ((Float_t)E1U1_matchcnt)/(ENT);
    E1U2_matchperc = ((Float_t)E1U2_matchcnt)/(ENT);
    E1D1_matchperc = ((Float_t)E1D1_matchcnt)/(ENT);
    E1D2_matchperc = ((Float_t)E1D2_matchcnt)/(ENT);
    E2U1_matchperc = ((Float_t)E2U1_matchcnt)/(ENT);
    E2U2_matchperc = ((Float_t)E2U2_matchcnt)/(ENT);
    E2D1_matchperc = ((Float_t)E2D1_matchcnt)/(ENT);
    E2D2_matchperc = ((Float_t)E2D2_matchcnt)/(ENT);
    W1U1_matchperc = ((Float_t)W1U1_matchcnt)/(ENT);
    W1U2_matchperc = ((Float_t)W1U2_matchcnt)/(ENT);
    W1D1_matchperc = ((Float_t)W1D1_matchcnt)/(ENT);
    W1D2_matchperc = ((Float_t)W1D2_matchcnt)/(ENT);
    W2U1_matchperc = ((Float_t)W2U1_matchcnt)/(ENT);
    W2U2_matchperc = ((Float_t)W2U2_matchcnt)/(ENT);
    W2D1_matchperc = ((Float_t)W2D1_matchcnt)/(ENT);
    W2D2_matchperc = ((Float_t)W2D2_matchcnt)/(ENT);

    printf("E1U1 match: %.2f%%\n",E1U1_matchperc*100);
    printf("E1U2 match: %.2f%%\n",E1U2_matchperc*100);
    printf("E1D1 match: %.2f%%\n",E1D1_matchperc*100);
    printf("E1D2 match: %.2f%%\n",E1D2_matchperc*100);
    printf("E2U1 match: %.2f%%\n",E2U1_matchperc*100);
    printf("E2U2 match: %.2f%%\n",E2U2_matchperc*100);
    printf("E2D1 match: %.2f%%\n",E2D1_matchperc*100);
    printf("E2D2 match: %.2f%%\n",E2D2_matchperc*100);
    printf("W1U1 match: %.2f%%\n",W1U1_matchperc*100);
    printf("W1U2 match: %.2f%%\n",W1U2_matchperc*100);
    printf("W1D1 match: %.2f%%\n",W1D1_matchperc*100);
    printf("W1D2 match: %.2f%%\n",W1D2_matchperc*100);
    printf("W2U1 match: %.2f%%\n",W2U1_matchperc*100);
    printf("W2U2 match: %.2f%%\n",W2U2_matchperc*100);
    printf("W2D1 match: %.2f%%\n",W2D1_matchperc*100);
    printf("W2D2 match: %.2f%%\n",W2D2_matchperc*100);
  };


  /*
  gSystem->RedirectOutput("match.log");
  printf("%d %d %f %f %f %f\n",runnum,segnum,EOR_matchperc,WOR_matchperc,ET_matchperc,IT_matchperc);
  gSystem->RedirectOutput(0);
  */
};
