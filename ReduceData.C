// reads TwoTr from Output file and extracts
// only the essential subset for spin analysis,
// adding on the relative luminosities for each run
// -- see code under comment "reduction cut"
//    -- (e.g., N12==2 events with  specific E and Pt range)
// -- writes output to redset/
//
// 145ka

void ReduceData(const char * filename="Outputset080ac.root",
                const char * dirname="../../Output")
{
  // load polarization and rellum data
  gSystem->Load("src/RunInfo.so");
  RunInfo * RD = new RunInfo();


  char root_file[256];
  sprintf(root_file,"%s/%s",dirname,filename);
  printf("opening %s\n",root_file);
  TFile * infile = new TFile(root_file,"READ");
  TTree * twotr = (TTree*) infile->Get("TwoTr");

  char outname[256];
  sprintf(outname,"redset/Red%s",filename);
  printf("reducing to data set %s\n",outname);
  TFile * outfile = new TFile(outname,"RECREATE");
  TTree * str = new TTree("str","str");

  Int_t ent = twotr->GetEntries();
  printf(" TwoTr->GetEntries() = %d\n",ent);

  Float_t M12,N12,E12,Phi,Eta,Pt,Z;
  Int_t spin,TrigBits,runnum,fill,Bunchid7bit;
  Bool_t kicked_str;
  Int_t blue_str,yell_str,spin_str;
  Float_t b_pol_str,y_pol_str;
  //Float_t R_bbc[10];
  //Float_t R_zdc[10];
  //Float_t R_vpd[10];
  //Float_t R_bbc_err[10];
  //Float_t R_zdc_err[10];
  //Float_t R_vpd_err[10];
  //Int_t pattern;
  Bool_t isConsistent;
  Float_t b_pol;
  Float_t y_pol;
  Float_t b_pol_err;
  Float_t y_pol_err;
  Int_t L2sum[2];
  UInt_t lastdsm[8];
  Short_t RPE_QTN;
  Short_t RPW_QTN;
  Short_t RPE_Idx[16]; // [rp chan]
  Short_t RPE_TAC[16];
  Short_t RPE_ADC[16];
  Short_t RPW_Idx[16];
  Short_t RPW_TAC[16];
  Short_t RPW_ADC[16];
  Float_t RPvertex;
  Char_t BBCE_QTN;
  Char_t BBCW_QTN;
  Char_t BBCE_Idx[16];
  Short_t BBCE_TAC[16];
  Short_t BBCE_ADC[16];
  Char_t BBCW_Idx[16];
  Short_t BBCW_TAC[16];
  Short_t BBCW_ADC[16];
  Float_t BBCvertex;

  // RP MuDST branches
  Int_t n_tracks,n_trackpoints;
  // track variables -- prefixed with t_
  Int_t t_index[n_tracks_max];
  Int_t t_branch[n_tracks_max];
  Int_t t_type[n_tracks_max];
  Int_t t_planesUsed[n_tracks_max];
  Double_t t_p[n_tracks_max];
  Double_t t_pt[n_tracks_max];
  Double_t t_eta[n_tracks_max];
  Double_t t_time[n_tracks_max];
  Double_t t_theta[n_tracks_max];
  Double_t t_thetaRP[n_tracks_max];
  Double_t t_phi[n_tracks_max];
  Double_t t_phiRP[n_tracks_max];
  Double_t t_t[n_tracks_max];
  Double_t t_xi[n_tracks_max];
  Bool_t t_gold[n_tracks_max];
  // trackpoint variables -- prefixed with p0_ and p1_
  Bool_t p_tpExists[2][n_tracks_max];
  Int_t p_RPid[2][n_tracks_max];
  Int_t p_quality[2][n_tracks_max];
  Double_t p_x[2][n_tracks_max];
  Double_t p_y[2][n_tracks_max];
  Double_t p_z[2][n_tracks_max];

  twotr->SetBranchAddress("M12",&M12);
  twotr->SetBranchAddress("N12",&N12);
  twotr->SetBranchAddress("E12",&E12);
  twotr->SetBranchAddress("Z",&Z);
  twotr->SetBranchAddress("Phi",&Phi);
  twotr->SetBranchAddress("Eta",&Eta);
  twotr->SetBranchAddress("Pt",&Pt);
  twotr->SetBranchAddress("spin",&spin);
  twotr->SetBranchAddress("TrigBits",&TrigBits);
  twotr->SetBranchAddress("Rnum",&runnum);
  twotr->SetBranchAddress("Bunchid7bit",&Bunchid7bit);
  twotr->SetBranchAddress("L2sum",L2sum);
  twotr->SetBranchAddress("lastdsm",lastdsm);
  
  /*
  twotr->SetBranchAddress("RPE_QTN",&RPE_QTN);
  twotr->SetBranchAddress("RPW_QTN",&RPW_QTN);
  twotr->SetBranchAddress("RPE_Idx",RPE_Idx);
  twotr->SetBranchAddress("RPE_TAC",RPE_TAC);
  twotr->SetBranchAddress("RPE_ADC",RPE_ADC);
  twotr->SetBranchAddress("RPW_Idx",RPW_Idx);
  twotr->SetBranchAddress("RPW_TAC",RPW_TAC);
  twotr->SetBranchAddress("RPW_ADC",RPW_ADC);
  twotr->SetBranchAddress("RPvertex",&RPvertex);

  twotr->SetBranchAddress("QTNE",&BBCE_QTN);
  twotr->SetBranchAddress("QTNW",&BBCW_QTN);
  twotr->SetBranchAddress("QTEBBCInd",BBCE_Idx);
  twotr->SetBranchAddress("QTEBBCTAC",BBCE_TAC);
  twotr->SetBranchAddress("QTEBBCADC",BBCE_ADC);
  twotr->SetBranchAddress("QTWBBCInd",BBCW_Idx);
  twotr->SetBranchAddress("QTWBBCTAC",BBCW_TAC);
  twotr->SetBranchAddress("QTWBBCADC",BBCW_ADC);
  twotr->SetBranchAddress("QTBVertex",&BBCvertex);
  */

  // mudst branches
  twotr->SetBranchAddress("RP_n_tracks",&n_tracks);
  twotr->SetBranchAddress("RP_n_trackpoints",&n_trackpoints);
  // tracks
  twotr->SetBranchAddress("RP_t_index",t_index); // track index number
  twotr->SetBranchAddress("RP_t_branch",t_branch); // RP branch (0=EU 1=ED 2=WU 3=WD)
  twotr->SetBranchAddress("RP_t_type",t_type); // track type (see next line)
    /* 0=rpsLocal -- 1 track point
     * 1=rpsGlobal -- 2 track points
     * 2=rpsUndefined -- track not defined
     */
  twotr->SetBranchAddress("RP_t_planesUsed",t_planesUsed); // number of SSD planes hit by track points in track
  twotr->SetBranchAddress("RP_t_p",t_p); // momentum
  twotr->SetBranchAddress("RP_t_pt",t_pt); // transverse momentum
  twotr->SetBranchAddress("RP_t_eta",t_eta); // pseudorapidity
  twotr->SetBranchAddress("RP_t_time",t_time); // time of track detection
  twotr->SetBranchAddress("RP_t_theta",t_theta); // polar angle at RP according to STAR coord sys
  twotr->SetBranchAddress("RP_t_thetaRP",t_thetaRP); // polar angle at RP according to STAR survey
  twotr->SetBranchAddress("RP_t_phi",t_phi); // azimuth at RP according to STAR coord sys
  twotr->SetBranchAddress("RP_t_phiRP",t_phiRP); // azimuth at RP according to STAR survey
  twotr->SetBranchAddress("RP_t_t",t_t); // squared 4-momentum transfer
  twotr->SetBranchAddress("RP_t_xi",t_xi); // fractional momentum loss (pbeam-p)/pbeam
  twotr->SetBranchAddress("RP_t_gold",t_gold); // my track quality variable (2 track points in all 2x4=8 Si planes)
  // track point 0
  twotr->SetBranchAddress("RP_p0_tpExists",p_tpExists[0]); // true if track point 0 exists
  twotr->SetBranchAddress("RP_p0_RPid",p_RPid[0]); // RP id (see next line)
    /* 0=E1U  1=E1D  2=E2U  3=E2D
     * 4=W1U  5=W1D  6=W2U  7=W2D
     */
  twotr->SetBranchAddress("RP_p0_quality",p_quality[0]); // track point quality (see next line)
    /* 0=rpsNormal -- not golden and not undefined
     * 1=rpsGolden -- single cluster in all 4 SSD planes
     * 2=rpsNotSet -- undefined track point
     */
  twotr->SetBranchAddress("RP_p0_x",p_x[0]); // STAR survey coords x-position
  twotr->SetBranchAddress("RP_p0_y",p_y[0]); // STAR survey coords y-position
  twotr->SetBranchAddress("RP_p0_z",p_z[0]); // STAR survey coords z-position
  // track point 1
  twotr->SetBranchAddress("RP_p1_tpExists",p_tpExists[1]); // true if track point 1 exists
  twotr->SetBranchAddress("RP_p1_RPid",p_RPid[1]);
  twotr->SetBranchAddress("RP_p1_quality",p_quality[1]);
  twotr->SetBranchAddress("RP_p1_x",p_x[1]);
  twotr->SetBranchAddress("RP_p1_y",p_y[1]);
  twotr->SetBranchAddress("RP_p1_z",p_z[1]);


  str->Branch("M12",&M12,"M12/F");
  str->Branch("N12",&N12,"N12/F");
  str->Branch("E12",&E12,"E12/F");
  str->Branch("Z",&Z,"Z/F");
  str->Branch("Phi",&Phi,"Phi/F");
  str->Branch("Eta",&Eta,"Eta/F");
  str->Branch("Pt",&Pt,"Pt/F");
  str->Branch("spin",&spin_str,"spin/I"); // spinbit; set to 40 if kicked or abort/empty
  str->Branch("blue",&blue_str,"blue/I");
  str->Branch("yell",&yell_str,"yell/I");
  str->Branch("kicked",&kicked_str,"kicked/O");
  str->Branch("Bunchid7bit",&Bunchid7bit,"Bunchid7bit/I");
  str->Branch("TrigBits",&TrigBits,"TrigBits/I");
  str->Branch("runnum",&runnum,"runnum/I");
  str->Branch("fill",&fill,"fill/I");
  str->Branch("L2sum",L2sum,"L2sum[2]/i");
  str->Branch("lastdsm",lastdsm,"lastdsm[8]/i");

  /*
  str->Branch("RPE_QTN",&RPE_QTN,"RPE_QTN/S");
  str->Branch("RPW_QTN",&RPW_QTN,"RPW_QTN/S");
  str->Branch("RPE_Idx",RPE_Idx,"RPE_Idx[RPE_QTN]/S");
  str->Branch("RPE_TAC",RPE_TAC,"RPE_TAC[RPE_QTN]/S");
  str->Branch("RPE_ADC",RPE_ADC,"RPE_ADC[RPE_QTN]/S");
  str->Branch("RPW_Idx",RPW_Idx,"RPW_Idx[RPW_QTN]/S");
  str->Branch("RPW_TAC",RPW_TAC,"RPW_TAC[RPW_QTN]/S");
  str->Branch("RPW_ADC",RPW_ADC,"RPW_ADC[RPW_QTN]/S");
  str->Branch("RPvertex",&RPvertex,"RPvertex/F");

  str->Branch("BBCE_QTN",&BBCE_QTN,"BBCE_QTN/B");
  str->Branch("BBCW_QTN",&BBCW_QTN,"BBCW_QTN/B");
  str->Branch("BBCE_Idx",BBCE_Idx,"BBCE_Idx[BBCE_QTN]/B");
  str->Branch("BBCE_TAC",BBCE_TAC,"BBCE_TAC[BBCE_QTN]/S");
  str->Branch("BBCE_ADC",BBCE_ADC,"BBCE_ADC[BBCE_QTN]/S");
  str->Branch("BBCW_Idx",BBCW_Idx,"BBCW_Idx[BBCW_QTN]/B");
  str->Branch("BBCW_TAC",BBCW_TAC,"BBCW_TAC[BBCW_QTN]/S");
  str->Branch("BBCW_ADC",BBCW_ADC,"BBCW_ADC[BBCW_QTN]/S");
  str->Branch("BBCvertex",&BBCvertex,"BBCvertex/F");
  */

  // mudst branches
  str->Branch("RP_n_tracks",&n_tracks,"RP_n_tracks/I");
  str->Branch("RP_n_trackpoints",&n_trackpoints,"RP_n_trackpoints/I");
  str->Branch("RP_t_index",t_index,"RP_t_index[RP_n_tracks]/I");
  str->Branch("RP_t_branch",t_branch,"RP_t_branch[RP_n_tracks]/I");
  str->Branch("RP_t_type",t_type,"RP_t_type[RP_n_tracks]/I");
  str->Branch("RP_t_planesUsed",t_planesUsed,"RP_t_planesUsed[RP_n_tracks]/I");
  str->Branch("RP_t_p",t_p,"RP_t_p[RP_n_tracks]/D");
  str->Branch("RP_t_pt",t_pt,"RP_t_pt[RP_n_tracks]/D");
  str->Branch("RP_t_eta",t_eta,"RP_t_eta[RP_n_tracks]/D");
  str->Branch("RP_t_time",t_time,"RP_t_time[RP_n_tracks]/D");
  str->Branch("RP_t_theta",t_theta,"RP_t_theta[RP_n_tracks]/D");
  str->Branch("RP_t_thetaRP",t_thetaRP,"RP_t_thetaRP[RP_n_tracks]/D");
  str->Branch("RP_t_phi",t_phi,"RP_t_phi[RP_n_tracks]/D");
  str->Branch("RP_t_phiRP",t_phiRP,"RP_t_phiRP[RP_n_tracks]/D");
  str->Branch("RP_t_t",t_t,"RP_t_t[RP_n_tracks]/D");
  str->Branch("RP_t_xi",t_xi,"RP_t_xi[RP_n_tracks]/D");
  str->Branch("RP_t_gold",t_gold,"RP_t_gold[RP_n_tracks]/O");
  str->Branch("RP_p0_tpExists",p_tpExists[0],"RP_p0_tpExists[RP_n_tracks]/O");
  str->Branch("RP_p0_RPid",p_RPid[0],"RP_p0_RPid[RP_n_tracks]/I");
  str->Branch("RP_p0_quality",p_quality[0],"RP_p0_quality[RP_n_tracks]/I");
  str->Branch("RP_p0_x",p_x[0],"RP_p0_x[RP_n_tracks]/D");
  str->Branch("RP_p0_y",p_y[0],"RP_p0_y[RP_n_tracks]/D");
  str->Branch("RP_p0_z",p_z[0],"RP_p0_z[RP_n_tracks]/D");
  str->Branch("RP_p1_tpExists",p_tpExists[1],"RP_p1_tpExists[RP_n_tracks]/O");
  str->Branch("RP_p1_RPid",p_RPid[1],"RP_p1_RPid[RP_n_tracks]/I");
  str->Branch("RP_p1_quality",p_quality[1],"RP_p1_quality[RP_n_tracks]/I");
  str->Branch("RP_p1_x",p_x[1],"RP_p1_x[RP_n_tracks]/D");
  str->Branch("RP_p1_y",p_y[1],"RP_p1_y[RP_n_tracks]/D");
  str->Branch("RP_p1_z",p_z[1],"RP_p1_z[RP_n_tracks]/D");


  /*
  char R_bbc_n[10][32];
  char R_zdc_n[10][32];
  char R_vpd_n[10][32];
  char R_bbc_err_n[10][32];
  char R_zdc_err_n[10][32];
  char R_vpd_err_n[10][32];
  char R_bbc_d[10][32];
  char R_zdc_d[10][32];
  char R_vpd_d[10][32];
  char R_bbc_err_d[10][32];
  char R_zdc_err_d[10][32];
  char R_vpd_err_d[10][32];
  for(Int_t r=1; r<10; r++)
  {
    sprintf(R_bbc_n[r],"R%d_bbc",r);
    sprintf(R_zdc_n[r],"R%d_zdc",r);
    sprintf(R_vpd_n[r],"R%d_vpd",r);
    sprintf(R_bbc_err_n[r],"R%d_bbc_err",r);
    sprintf(R_zdc_err_n[r],"R%d_zdc_err",r);
    sprintf(R_vpd_err_n[r],"R%d_vpd_err",r);

    sprintf(R_bbc_d[r],"R%d_bbc/F",r);
    sprintf(R_zdc_d[r],"R%d_zdc/F",r);
    sprintf(R_vpd_d[r],"R%d_vpd/F",r);
    sprintf(R_bbc_err_d[r],"R%d_bbc_err/F",r);
    sprintf(R_zdc_err_d[r],"R%d_zdc_err/F",r);
    sprintf(R_vpd_err_d[r],"R%d_vpd_err/F",r);

    str->Branch(R_bbc_n[r],&(R_bbc[r]),R_bbc_d[r]);
    str->Branch(R_bbc_err_n[r],&(R_bbc_err[r]),R_bbc_err_d[r]);
    str->Branch(R_zdc_n[r],&(R_zdc[r]),R_zdc_d[r]);
    str->Branch(R_zdc_err_n[r],&(R_zdc_err[r]),R_zdc_err_d[r]);
    str->Branch(R_vpd_n[r],&(R_vpd[r]),R_vpd_d[r]);
    str->Branch(R_vpd_err_n[r],&(R_vpd_err[r]),R_vpd_err_d[r]);
  };

  str->Branch("isConsistent",&isConsistent,"isConsistent/O");
  str->Branch("b_pol",&b_pol,"b_pol/F");
  str->Branch("y_pol",&y_pol,"y_pol/F");
  str->Branch("b_pol_err",&b_pol_err,"b_pol_err/F");
  str->Branch("y_pol_err",&y_pol_err,"y_pol_err/F");
  str->Branch("pattern",&pattern,"pattern/I");
  */

  
  // reduction loop
  Int_t runnum_tmp=0;
  for(Int_t q=0; q<twotr->GetEntries(); q++)
  {
    twotr->GetEntry(q);

    if(q%1000 == 0) printf("%d/%d (str ent=%d)\n",q+1,twotr->GetEntries(),str->GetEntries());

    // pi0 only reduction cut
    //if(N12==2 && fabs(M12-0.15)<0.15 && (TrigBits&0xFFF)>0)

    // reduction cut
    if(M12>=0)
    {
      if(runnum!=runnum_tmp)
      {

        /*
        for(Int_t r=1; r<10; r++)
        {
          R_bbc[r] = RD->Rellum(runnum,r,"bbc");
          R_zdc[r] = RD->Rellum(runnum,r,"zdc");
          R_vpd[r] = RD->Rellum(runnum,r,"vpd");

          R_bbc_err[r] = RD->RellumErr(runnum,r,"bbc");
          R_zdc_err[r] = RD->RellumErr(runnum,r,"zdc");
          R_vpd_err[r] = RD->RellumErr(runnum,r,"vpd");
        };
        pattern = RD->Pattern(runnum);
        */

        b_pol = RD->BluePol(runnum);
        y_pol = RD->YellPol(runnum);
        b_pol_err = RD->BluePolErr(runnum);
        y_pol_err = RD->YellPolErr(runnum);

        isConsistent = RD->RellumConsistent(runnum);

        fill = RD->GetFill(runnum);
        runnum_tmp = runnum;
      };

      blue_str = RD->BlueSpin(runnum,Bunchid7bit);
      yell_str = RD->YellSpin(runnum,Bunchid7bit);
      kicked_str = RD->Kicked(runnum,Bunchid7bit);


      // add entry to tree iff there exists rellum and polarization data
      // AND if relative luminosity passed consistency check
      if(fill!=0 && b_pol>0 && y_pol>0 && isConsistent)
      {
        if(blue_str==-1 && yell_str==-1) spin_str = 0;
        else if(blue_str==-1 && yell_str==1) spin_str = 1;
        else if(blue_str==1 && yell_str==-1) spin_str = 2;
        else if(blue_str==1 && yell_str==1) spin_str = 3;
        else spin_str=40;
        if(kicked_str)
        {
          spin=40;
          blue_str=0;
          yell_str=0;
        };

        str->Fill();
      };
    };
  };

  str->Write("str");
}
