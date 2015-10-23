void CombineOverlaps(TString caseToConsider="scint",TString fmsTrg="")
{
  const Int_t MAX_RUNS = 12; // assumed max number of runs in a set file

  // class instantiation
  gSystem->Load("src/RunInfo.so");
  RunInfo * RD = new RunInfo();
  LevelTwo * T = new LevelTwo();
  Environ * env = new Environ();
  EventClass * ev = new EventClass();
  TriggerBoolean * trg_bool = 
    new TriggerBoolean(env->STG1,env->STG2,env->MIPN,env->USE_TCU_BITS);

  int i,j;
  enum ew_enum {kE,kW};
  enum io_enum {kI,kO};
  enum ud_enum {kU,kD};
  enum ns_enum {kN,kS};
  enum lr_enum {kL,kR};

  // number of trigger booleans & boolean indices
  Int_t NBOOL_tmp = trg_bool->NBOOL;
  const Int_t NBOOL = NBOOL_tmp;
  Int_t unbiased_bool;
  Int_t et_bool,sd_bool;
  for(i=0; i<NBOOL; i++)
  {
    if(!strcmp(trg_bool->Name(i),"N")) unbiased_bool=i;
    if(!strcmp(trg_bool->Name(i),"ET")) et_bool=i;
    if(!strcmp(trg_bool->Name(i),"SDOR")) sd_bool=i;
  };
  printf("boolean indices: N:%d ET:%d SDOR:%d\n",unbiased_bool,et_bool,sd_bool);

  // obtain trigger boolean strength cases to analyse
  // -- case number = index for each case
  TTree * bool_case_tr = new TTree();
  bool_case_tr->ReadFile("boolean_cases.dat","stg1/I:stg2/I:mipn/I:use_tcu/I:case_name/C");
  Int_t stg1_r,stg2_r,mipn_r,use_tcu_r;
  Int_t tcu_case_num=-1;
  Int_t case_num=-1;
  char case_name_r[64];
  bool_case_tr->SetBranchAddress("stg1",&stg1_r);
  bool_case_tr->SetBranchAddress("stg2",&stg2_r);
  bool_case_tr->SetBranchAddress("mipn",&mipn_r);
  bool_case_tr->SetBranchAddress("use_tcu",&use_tcu_r);
  bool_case_tr->SetBranchAddress("case_name",case_name_r);
  Int_t NCASE_tmp = bool_case_tr->GetEntries();
  const Int_t NCASE = NCASE_tmp;
  Int_t STG1_case[NCASE]; // [case number]
  Int_t STG2_case[NCASE];
  Int_t MIPN_case[NCASE];
  Int_t USE_TCU_BITS_case[NCASE];
  TString str_case[NCASE];
  TString case_name[NCASE];
  for(i=0; i<NCASE; i++)
  {
    bool_case_tr->GetEntry(i);
    STG1_case[i] = stg1_r;
    STG2_case[i] = stg2_r;
    MIPN_case[i] = mipn_r;
    USE_TCU_BITS_case[i] = use_tcu_r;
    case_name[i] = Form("%s",case_name_r);
    str_case[i] = Form("%d%d%d%d",STG1_case[i],STG2_case[i],MIPN_case[i],USE_TCU_BITS_case[i]);
    if(!strcmp(case_name_r,caseToConsider.Data())) case_num=i;
    else if(!strcmp(case_name_r,"tcu")) tcu_case_num=i;
  };
  if(case_num<0) { fprintf(stderr,"ERROR: caseToConsider invalid\n"); return; }
  else if(tcu_case_num<0) { fprintf(stderr,"ERROR: no TCU case found\n"); return; }


  // load RP runlog tree
  TFile * ptr_file = new TFile("rptrg15/pvsi_plots.root","READ");
  TTree * ptr = (TTree*) ptr_file->Get("ptr");
  Int_t runnum_p;
  Double_t P_SD,P_ET;
  ptr->SetBranchAddress("runnum",&runnum_p);
  ptr->SetBranchAddress("Prob_RP_SD",&P_SD);
  ptr->SetBranchAddress("Prob_RP_ET",&P_ET);



  // build array of overlapset/*.root TFile pointers
  const Int_t MAX_NUM_FILES=200;
  TFile * ove_file[MAX_NUM_FILES]; 
  Int_t ove_file_cnt=0;
  char ls_str[256];
  if(fmsTrg=="") strcpy(ls_str,"");
  else sprintf(ls_str,"_%s",fmsTrg.Data());
  sprintf(ls_str,".! ls overlapset%s/OverlapSet*.root > ove_files.txt",ls_str);
  printf("%s\n",ls_str);
  gROOT->ProcessLine(ls_str);
  const Int_t filename_buffer=64;
  char filename[MAX_NUM_FILES][filename_buffer];
  char temp[filename_buffer];
  FILE * ove_files;
  ove_files = fopen("ove_files.txt","r");
  if(ove_files==NULL)
  {
    fprintf(stderr,"Error opening ove_files.txt\n");
    return;
  }
  else
  {
    while(!feof(ove_files))
    {
      fgets(filename[ove_file_cnt],filename_buffer,ove_files);

      // fgets reads in "returns"; this hack gets rid of them
      sscanf(filename[ove_file_cnt],"%s",filename[ove_file_cnt]);

      if(strcmp(filename[ove_file_cnt],""))
      {
        //printf("%d: %s\n",ove_file_cnt,filename[ove_file_cnt]);
        ove_file[ove_file_cnt] = new TFile(filename[ove_file_cnt],"READ");
        ove_file_cnt++;
      };
    };
  };
  const Int_t NFILES=ove_file_cnt;
  gROOT->ProcessLine(".! rm ove_files.txt");
  printf("NFILES=%d\n",NFILES);


  // open object arrays and determine run numbers list
  const Int_t MAX_NUM_RUNS=1000;
  Int_t runnum_arr[MAX_NUM_RUNS];
  Int_t sorted[MAX_NUM_RUNS];
  Int_t plotcoord[MAX_NUM_RUNS][2]; // [run index] [0=TFile num / 1=TObjArray index]
  Int_t run_cnt=0;
  for(int r=0; r<MAX_NUM_RUNS; r++) 
  {
    runnum_arr[r] = 99999999;
    plotcoord[r][0] = -1;
    plotcoord[r][1] = -1;
  };
  TObjArray * bool_w_case_arr[NFILES];
  TObjArray * bool_prob_case_arr[NFILES];
  TObjArray * frac_arr[NFILES];
  TString frac_arr_n = "tcu_frac_"+caseToConsider+"_arr";
  Int_t entz;
  for(int x=0; x<NFILES; x++)
  {
    bool_w_case_arr[x] = (TObjArray*) ove_file[x]->Get("bool_w_case_arr");
    bool_prob_case_arr[x] = (TObjArray*) ove_file[x]->Get("bool_prob_case_arr");
    frac_arr[x] = (TObjArray*) ove_file[x]->Get(frac_arr_n.Data());
    printf("%p %p %p\n",(void*)bool_w_case_arr[x],(void*)bool_prob_case_arr[x],(void*)frac_arr[x]);
    entz = bool_w_case_arr[x]->GetEntries();
    printf("entz=%d\n",entz);
    for(int y=0; y<bool_w_case_arr[x]->GetEntries(); y++)
    {
      if(run_cnt<MAX_NUM_RUNS)
      {
        sscanf(((TH2D*)(bool_w_case_arr[x]->At(y)))->GetName(),"bool_w_case_r%d",&(runnum_arr[run_cnt]));
        plotcoord[run_cnt][0] = x;
        plotcoord[run_cnt][1] = y;
        run_cnt++;
      }
      else 
      {
        fprintf(stderr,"ERROR: MAX_NUM_RUNS is too small\n");
        return;
      };
    };
  };


  // sort list of run numbers (probably already sorted, but no harm in doing it anyway)
  // -- the array "sorted[]" contains indices for the sorted list of run numbers; one can 
  //    then use the array plotcoord to access specific plots for each TOjbArray 
  TMath::Sort(MAX_NUM_RUNS,runnum_arr,sorted,false);
  for(int r=0; r<MAX_NUM_RUNS; r++)
  {
    printf("%d %d %d %d %d\n",r,runnum_arr[r],runnum_arr[sorted[r]],plotcoord[sorted[r]][0],plotcoord[sorted[r]][1]);
  };
  printf("run_cnt=%d\n",run_cnt);




  // make plots
  // O is from entries in the bool_prob_case
  // X is from diagonal entry in the frac_arr

  TGraph * X_gr[NBOOL]; // fraction of TCU booleans which satisfy caseToConsider booleans vs. run index
  TGraph * O_tcu_gr[NBOOL]; // overlap of TCU boolean and FMS-OR boolean vs. run index
  TGraph * O_case_gr[NBOOL]; // overlap of caseToConsider boolean and FMS-OR boolean vs. run index
  TGraph * P_ET_gr; // number of RP_ET (=TCU ET) triggers per bXing vs. run index
  TGraph * P_SD_gr; // number of RP_SD (=TCU EOR || WOR) triggers per bXing vs. run index
  TGraph * XP_ET_gr; // scale P_ET by X_ET (fraction of TCU bools which satisfy caseToConsider) vs. run index
  TGraph * XP_SD_gr; // scale P_SD by X_SD (fraction of TCU bools which satisfy caseToConsider) vs. run index
  TGraph * OP_ET_tcu_gr; // ratio O_ET to P_ET via TCU vs. run index
  TGraph * OP_SD_tcu_gr; // ratio O_ET to P_ET via TCU vs. run index
  TGraph * OP_ET_case_gr; // ratio O_ET to P_ET via caseToConsider vs. run index
  TGraph * OP_SD_case_gr; // ratio O_ET to P_ET via caseToConsider vs. run index

  TString X_gr_n[NBOOL]; 
  TString O_tcu_gr_n[NBOOL]; 
  TString O_case_gr_n[NBOOL]; 
  TString X_gr_t[NBOOL]; 
  TString O_tcu_gr_t[NBOOL]; 
  TString O_case_gr_t[NBOOL]; 
  TString fmsTrg_t;
  TString XP_ET_gr_n;
  TString XP_SD_gr_n;
  TString XP_ET_gr_t;
  TString XP_SD_gr_t;
  TString OP_ET_tcu_gr_n;
  TString OP_SD_tcu_gr_n;
  TString OP_ET_tcu_gr_t;
  TString OP_SD_tcu_gr_t;
  TString OP_ET_case_gr_n;
  TString OP_SD_case_gr_n;
  TString OP_ET_case_gr_t;
  TString OP_SD_case_gr_t;

  if(fmsTrg=="") fmsTrg_t="FMS trig";
  else if(fmsTrg=="All") fmsTrg_t="FMSOR";
  else fmsTrg_t=fmsTrg;

  for(int b=0; b<NBOOL; b++)
  {
    X_gr_n[b] = Form("X_%s",trg_bool->Name(b));
    O_tcu_gr_n[b] = Form("O_TCU_%s",trg_bool->Name(b));
    O_case_gr_n[b] = Form("O_%s_%s",caseToConsider.Data(),trg_bool->Name(b));

    X_gr_t[b] = Form("fraction of TCU %s bools which satisfy %s %s bools (X^{%s}_{%s}) vs. run index",
        trg_bool->Name(b),caseToConsider.Data(),trg_bool->Name(b),caseToConsider.Data(),trg_bool->Name(b));
    O_tcu_gr_t[b] = Form("overlap of TCU %s bool and %s (O^{TCU}_{%s}) vs. run index",
        trg_bool->Name(b),fmsTrg_t.Data(),trg_bool->Name(b));
    O_case_gr_t[b] = Form("overlap of %s %s bool and %s (O^{%s}_{%s}) vs. run index",
        caseToConsider.Data(),trg_bool->Name(b),fmsTrg_t.Data(),caseToConsider.Data(),trg_bool->Name(b));

    X_gr[b] = new TGraph();
    O_tcu_gr[b] = new TGraph();
    O_case_gr[b] = new TGraph();

    X_gr[b]->SetName(X_gr_n[b].Data());
    O_tcu_gr[b]->SetName(O_tcu_gr_n[b].Data());
    O_case_gr[b]->SetName(O_case_gr_n[b].Data());

    X_gr[b]->SetTitle(X_gr_t[b].Data());
    O_tcu_gr[b]->SetTitle(O_tcu_gr_t[b].Data());
    O_case_gr[b]->SetTitle(O_case_gr_t[b].Data());

    X_gr[b]->SetMarkerStyle(kFullCircle);
    O_tcu_gr[b]->SetMarkerStyle(kFullCircle);
    O_case_gr[b]->SetMarkerStyle(kFullCircle);
    
    X_gr[b]->SetMarkerColor(kOrange+7);
    O_tcu_gr[b]->SetMarkerColor(kBlue-4);
    O_case_gr[b]->SetMarkerColor(kRed);
  };

  P_ET_gr = new TGraph();
  P_SD_gr = new TGraph();
  P_ET_gr->SetName("P_ET");
  P_SD_gr->SetName("P_SD");
  P_ET_gr->SetTitle("probability of RP_ET per bXing (P_{ET}) vs. run index");
  P_SD_gr->SetTitle("probability of RP_SD per bXing (P_{SD}) vs. run index");
  P_ET_gr->SetMarkerStyle(kFullCircle);
  P_SD_gr->SetMarkerStyle(kFullCircle);
  P_ET_gr->SetMarkerColor(kBlack);
  P_SD_gr->SetMarkerColor(kBlack);

  XP_ET_gr_n = "XP_ET";
  XP_SD_gr_n = "XP_SD";
  XP_ET_gr_t = Form("X_{ET}^{%s}P_{ET} vs. run index",caseToConsider.Data());
  XP_SD_gr_t = Form("X_{SDOR}^{%s}P_{SD} vs. run index",caseToConsider.Data());

  XP_ET_gr = new TGraph();
  XP_SD_gr = new TGraph();
  XP_ET_gr->SetName(XP_ET_gr_n.Data());
  XP_SD_gr->SetName(XP_SD_gr_n.Data());
  XP_ET_gr->SetTitle(XP_ET_gr_t.Data());
  XP_SD_gr->SetTitle(XP_SD_gr_t.Data());
  XP_ET_gr->SetMarkerStyle(kFullCircle);
  XP_SD_gr->SetMarkerStyle(kFullCircle);
  XP_ET_gr->SetMarkerColor(kAzure);
  XP_SD_gr->SetMarkerColor(kAzure);

  OP_ET_tcu_gr_n = "OP_ET_TCU";
  OP_SD_tcu_gr_n = "OP_SD_TCU";
  OP_ET_tcu_gr_t = "O^{TCU}_{ET}/P_{ET} vs. run index";
  OP_SD_tcu_gr_t = "O^{TCU}_{SDOR}/P_{SD} vs. run index";

  OP_ET_tcu_gr = new TGraph();
  OP_SD_tcu_gr = new TGraph();
  OP_ET_tcu_gr->SetName(OP_ET_tcu_gr_n.Data());
  OP_SD_tcu_gr->SetName(OP_SD_tcu_gr_n.Data());
  OP_ET_tcu_gr->SetTitle(OP_ET_tcu_gr_t.Data());
  OP_SD_tcu_gr->SetTitle(OP_SD_tcu_gr_t.Data());
  OP_ET_tcu_gr->SetMarkerStyle(kFullCircle);
  OP_SD_tcu_gr->SetMarkerStyle(kFullCircle);
  OP_ET_tcu_gr->SetMarkerColor(kOrange+2);
  OP_SD_tcu_gr->SetMarkerColor(kOrange+2);
  
  OP_ET_case_gr_n = Form("OP_ET_%s",caseToConsider.Data());
  OP_SD_case_gr_n = Form("OP_SD_%s",caseToConsider.Data());
  OP_ET_case_gr_t = Form("O^{%s}_{ET}/X_{ET}P_{ET} vs. run index",caseToConsider.Data());
  OP_SD_case_gr_t = Form("O^{%s}_{SDOR}/X_{SDOR}P_{SD} vs. run index",caseToConsider.Data());

  OP_ET_case_gr = new TGraph();
  OP_SD_case_gr = new TGraph();
  OP_ET_case_gr->SetName(OP_ET_case_gr_n.Data());
  OP_SD_case_gr->SetName(OP_SD_case_gr_n.Data());
  OP_ET_case_gr->SetTitle(OP_ET_case_gr_t.Data());
  OP_SD_case_gr->SetTitle(OP_SD_case_gr_t.Data());
  OP_ET_case_gr->SetMarkerStyle(kFullCircle);
  OP_SD_case_gr->SetMarkerStyle(kFullCircle);
  OP_ET_case_gr->SetMarkerColor(kOrange+2);
  OP_SD_case_gr->SetMarkerColor(kOrange+2);


  // fill graphs
  Float_t xx,oo_tcu,oo_case;
  Double_t pp_et,pp_sd;
  Int_t fi,ti,rr;
  Int_t binn;
  Bool_t run_found;
  Int_t pnt=0;
  for(int r=0; r<run_cnt; r++)
  {
    fi = plotcoord[sorted[r]][0];
    ti = plotcoord[sorted[r]][1];
    rr = runnum_arr[sorted[r]];
    run_found = false;

    for(int ptri=0; ptri<ptr->GetEntries(); ptri++)
    {
      ptr->GetEntry(ptri);
      if(runnum_p==rr)
      {
        run_found = true;
        pp_et = P_ET;
        pp_sd = P_SD;
        break;
      };
    };

    if(run_found)
    {
      for(int b=0; b<NBOOL; b++)
      {
        binn = ((TH2D*)(frac_arr[fi]->At(ti)))->GetBin(b+1,b+1);
        xx = ((TH2D*)(frac_arr[fi]->At(ti)))->GetBinContent(binn);
        X_gr[b]->SetPoint(pnt,r,xx);

        binn = ((TH2D*)(bool_prob_case_arr[fi]->At(ti)))->GetBin(tcu_case_num+1,b+1);
        oo_tcu = ((TH2D*)(bool_prob_case_arr[fi]->At(ti)))->GetBinContent(binn);
        O_tcu_gr[b]->SetPoint(pnt,r,oo_tcu);

        binn = ((TH2D*)(bool_prob_case_arr[fi]->At(ti)))->GetBin(case_num+1,b+1);
        oo_case = ((TH2D*)(bool_prob_case_arr[fi]->At(ti)))->GetBinContent(binn);
        O_case_gr[b]->SetPoint(pnt,r,oo_case);

        if(b==et_bool)
        {
          P_ET_gr->SetPoint(pnt,r,pp_et);
          XP_ET_gr->SetPoint(pnt,r,xx*pp_et);
          OP_ET_tcu_gr->SetPoint(pnt,r,oo_tcu/pp_et);
          OP_ET_case_gr->SetPoint(pnt,r,oo_case/(xx*pp_et));
        };
        if(b==sd_bool)
        {
          P_SD_gr->SetPoint(pnt,r,pp_sd);
          XP_SD_gr->SetPoint(pnt,r,xx*pp_sd);
          OP_SD_tcu_gr->SetPoint(pnt,r,oo_tcu/pp_sd);
          OP_SD_case_gr->SetPoint(pnt,r,oo_case/(xx*pp_sd));
        };
      };
      pnt++;
    };
  };

  
  // draw canvases
  TCanvas * c_OX_ET = new TCanvas("c_OX_ET","c_OX_ET",1000,1000);
  TCanvas * c_P_ET = new TCanvas("c_P_ET","c_P_ET",1000,1000);
  TCanvas * c_OP_ET = new TCanvas("c_OP_ET","c_OP_ET",1000,1000);
  TCanvas * c_OX_SD = new TCanvas("c_OX_SD","c_OX_SD",1000,1000);
  TCanvas * c_P_SD = new TCanvas("c_P_SD","c_P_SD",1000,1000);
  TCanvas * c_OP_SD = new TCanvas("c_OP_SD","c_OP_SD",1000,1000);

  c_OX_ET->Divide(1,3);
  c_OX_ET->cd(1); O_tcu_gr[et_bool]->Draw("AP");
  c_OX_ET->cd(2); O_case_gr[et_bool]->Draw("AP");
  c_OX_ET->cd(3); X_gr[et_bool]->Draw("AP");
  c_P_ET->Divide(1,2);
  c_P_ET->cd(1); P_ET_gr->Draw("AP");
  c_P_ET->cd(2); XP_ET_gr->Draw("AP");
  c_OP_ET->Divide(1,2);
  c_OP_ET->cd(1); OP_ET_tcu_gr->Draw("AP");
  c_OP_ET->cd(2); OP_ET_case_gr->Draw("AP");

  c_OX_SD->Divide(1,3);
  c_OX_SD->cd(1); O_tcu_gr[sd_bool]->Draw("AP");
  c_OX_SD->cd(2); O_case_gr[sd_bool]->Draw("AP");
  c_OX_SD->cd(3); X_gr[sd_bool]->Draw("AP");
  c_P_SD->Divide(1,2);
  c_P_SD->cd(1); P_SD_gr->Draw("AP");
  c_P_SD->cd(2); XP_SD_gr->Draw("AP");
  c_OP_SD->Divide(1,2);
  c_OP_SD->cd(1); OP_SD_tcu_gr->Draw("AP");
  c_OP_SD->cd(2); OP_SD_case_gr->Draw("AP");


  
  // write output
  TString outfile_n;
  if(fmsTrg=="") outfile_n = "rp_overlap.root";
  else outfile_n = Form("rp_overlap_%s.root",fmsTrg.Data());
  TFile * outfile = new TFile(outfile_n.Data(),"RECREATE");
  for(int b=0; b<NBOOL; b++)
  {
    X_gr[b]->Write();
    O_tcu_gr[b]->Write();
    O_case_gr[b]->Write();
  };
  P_ET_gr->Write();
  P_SD_gr->Write();
  XP_ET_gr->Write();
  XP_SD_gr->Write();
  OP_ET_tcu_gr->Write();
  OP_SD_tcu_gr->Write();
  OP_ET_case_gr->Write();
  OP_SD_case_gr->Write();

  c_OX_ET->Write();
  c_P_ET->Write();
  c_OP_ET->Write();
  c_OX_SD->Write();
  c_P_SD->Write();
  c_OP_SD->Write();
  
  cout << endl << outfile_n << " created" << endl;
};
