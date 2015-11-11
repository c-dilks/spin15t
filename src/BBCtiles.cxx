#include "BBCtiles.h"

namespace
{
  enum sl_enum {kS,kL};
  enum tpxyz_enum {kTile,kPMT,kX,kY,kZ};
  const Double_t sq = TMath::Sqrt(3);
};


BBCtiles::BBCtiles()
{
  // BBC coordinate map
  // (first define it here as constructor-local array)
  Int_t coord0[2][18][5] = 
  {
    {
      {  1,  1,  0,  1, -1 },
      {  2,  2,  1,  0, -1 },
      {  3,  3,  1, -1,  0 },
      {  4,  4,  0, -1,  1 },
      {  5,  5, -1,  0,  1 },
      {  6,  6, -1,  1,  0 },
      {  7,  7, -1,  2, -1 },
      {  8,  8,  0,  2, -2 },
      {  9,  7,  1,  1, -2 },
      { 10,  9,  2,  0, -2 },
      { 11, 10,  2, -1, -1 },
      { 12, 11,  2, -2,  0 },
      { 13, 12,  1, -2,  1 },
      { 14, 13,  0, -2,  2 },
      { 15, 12, -1, -1,  2 },
      { 16, 14, -2,  0,  2 },
      { 17, 15, -2,  1,  1 },
      { 18, 16, -2,  2,  0 }
    },
    {
      { 19, 17,  0,  1, -1 },
      { 20, 18,  1,  0, -1 },
      { 21, 18,  1, -1,  0 },
      { 22, 19,  0, -1,  1 },
      { 23, 20, -1,  0,  1 },
      { 24, 20, -1,  1,  0 },
      { 25, 21, -1,  2, -1 },
      { 26, 21,  0,  2, -2 },
      { 27, 21,  1,  1, -2 },
      { 28, 22,  2,  0, -2 },
      { 29, 22,  2, -1, -1 },
      { 30, 22,  2, -2,  0 },
      { 31, 23,  1, -2,  1 },
      { 32, 23,  0, -2,  2 },
      { 33, 23, -1, -1,  2 },
      { 34, 24, -2,  0,  2 },
      { 35, 24, -2,  1,  1 },
      { 36, 24, -2,  2,  0 }
    }
  };


  // copy constructor-local array to class member "coord"
  for(sl=0; sl<2; sl++)
  {
    for(tt=0; tt<18; tt++)
    {
      for(tpxyz=0; tpxyz<5; tpxyz++)
      {
        coord[sl][tt][tpxyz] = coord0[sl][tt][tpxyz];
      };
    };
  };


  // fill coord_pmt (pmt->tile number(s))
  Int_t pmt_cnt[25]; 
  for(pp=0; pp<=24; pp++) 
  {
    pmt_cnt[pp]=0; 
    for(int ppp=0; ppp<3; ppp++) coord_pmt[pp][ppp]=0;
  };
  for(tt=1; tt<=36; tt++)
  {
    coord_pmt[GetPMTOfTile(tt)][pmt_cnt[GetPMTOfTile(tt)]] = tt;
    (pmt_cnt[GetPMTOfTile(tt)])++;
  };


  // hexagon side size for event display
  cellsize[kS] = 1;
  cellsize[kL] = 4;

  // line colors
  hexlinecolor[kS] = (Int_t) kGreen+1;
  hexlinecolor[kL] = (Int_t) kAzure+1;


  // initialise event display
  for(sl=0; sl<2; sl++) 
  {
    initTlines = true;
    tile_poly[sl] = this->MakeNewPoly(sl);
    initTlines = false;
    pmt_poly[sl] = this->MakeNewPoly(sl);
    for(tt=0; tt<18; tt++)
    {
      tile_poly[sl]->SetBinContent(coord[sl][tt][kTile]-18*sl,coord[sl][tt][kTile]);
      pmt_poly[sl]->SetBinContent(coord[sl][tt][kTile]-18*sl,coord[sl][tt][kPMT]);
    };
    tile_poly[sl]->SetTitle("BBC tile numbers");
    pmt_poly[sl]->SetTitle("BBC PMT numbers");
  };


  // compute azimuths
  for(tt=1; tt<=36; tt++) azi_tile[tt] = ComputeAzimuthOfTile(tt);
  for(pp=1; pp<=24; pp++) azi_pmt[pp] = ComputeAveAzimuthOfPMT(pp);
};


TH2Poly * BBCtiles::MakeNewPoly(Int_t sl0)
{
  TH2Poly * hc = new TH2Poly();
  for(tt=0; tt<18; tt++)
  {
    xhex = coord[sl0][tt][kX];
    yhex = coord[sl0][tt][kY];
    zhex = coord[sl0][tt][kZ];

    HexToCart(cellsize[sl0],xhex,yhex,zhex,xcart,ycart);

    x[0] = xcart - cellsize[sl0]/2.0;
    x[1] = xcart + cellsize[sl0]/2.0;
    x[2] = xcart + cellsize[sl0];
    x[3] = x[1];
    x[4] = x[0];
    x[5] = xcart - cellsize[sl0];
    
    y[0] = ycart + cellsize[sl0]*sq/2.0;
    y[1] = y[0];
    y[2] = ycart;
    y[3] = ycart - cellsize[sl0]*sq/2.0;
    y[4] = y[3];
    y[5] = y[2];

    for(int kk=0; kk<6; kk++)
    {
      hexline[kk][sl0][tt] = new TLine(x[kk],y[kk],x[(kk+1)%6],y[(kk+1)%6]);
      hexline[kk][sl0][tt]->SetLineColor(hexlinecolor[sl0]);
      hexline[kk][sl0][tt]->SetLineWidth(2);
    };

    hc->AddBin(6, x, y);
  };
  return hc;
};


void BBCtiles::HexToCart(Double_t cellsize0, Int_t x0, Int_t y0, Int_t z0, Double_t &xx0, Double_t &yy0)
{
  xx0 = 1.5 * cellsize0 * x0;
  yy0 = TMath::Sqrt(3)/2.0 * cellsize0 * (y0-z0);
};


/* coordinate accessors */

// return TH2Poly bin number from tile number (to be used with GetSlOfTile)
Int_t BBCtiles::GetBinOfTile(Int_t tile0) { return tile0 - (tile0>18)*18; };

// return sl from tile number
Int_t BBCtiles::GetSlOfTile(Int_t tile0) { return (tile0>18) ? kL:kS; };

// return sl from pmt number
Int_t BBCtiles::GetSlOfPMT(Int_t pmt0) { return (pmt0>16) ? kL:kS; };

// return PMT from tile number
Int_t BBCtiles::GetPMTOfTile(Int_t tile0) { return coord[GetSlOfTile(tile0)][GetBinOfTile(tile0)-1][kPMT]; };

// return tile number(s) from PMT (use whichTile to pick which tile)
Int_t BBCtiles::GetTileOfPMT(Int_t pmt0, Int_t whichTile)
{
  if(whichTile>=0 && whichTile<3)
    return coord_pmt[pmt0][whichTile];
  else return 0;
};

// return tile azimuth
Double_t BBCtiles::GetAzimuthOfTile(Int_t tile0,Bool_t returnDeg)
{
  if(returnDeg==1) return azi_tile[tile0] / TMath::Pi() * 180.0;
  else return azi_tile[tile0];
};
Double_t BBCtiles::GetAveAzimuthOfPMT(Int_t pmt0) { return azi_pmt[pmt0]; };


// return hex coordinates from tile number
Int_t BBCtiles::GetXhexOfTile(Int_t tile0) { return coord[GetSlOfTile(tile0)][GetBinOfTile(tile0)-1][kX]; };
Int_t BBCtiles::GetYhexOfTile(Int_t tile0) { return coord[GetSlOfTile(tile0)][GetBinOfTile(tile0)-1][kY]; };
Int_t BBCtiles::GetZhexOfTile(Int_t tile0) { return coord[GetSlOfTile(tile0)][GetBinOfTile(tile0)-1][kZ]; };


// compute azimuthal angle
Double_t BBCtiles::ComputeAzimuthOfTile(Int_t tile0, Bool_t returnDeg)
{
  Double_t az;
  xhex = GetXhexOfTile(tile0);
  yhex = GetYhexOfTile(tile0);
  zhex = GetZhexOfTile(tile0);
  HexToCart(cellsize[GetSlOfTile(tile0)],xhex,yhex,zhex,xcart,ycart);
  az = TMath::ATan2(ycart,xcart);
  if(returnDeg==1) return az / TMath::Pi() * 180.0; // degrees
  else return az; // radians
};


// compute average azimuthal angle of PMT
Double_t BBCtiles::ComputeAveAzimuthOfPMT(Int_t pmt0)
{
  Int_t tile_current;
  Double_t az_tmp;
  Double_t cos_az=0;
  Double_t sin_az=0;
  Int_t tile_count=0;

  for(int tl=0; tl<3; tl++)
  {
    tile_current = GetTileOfPMT(pmt0,tl);
    if(tile_current>0)
    {
      az_tmp = ComputeAzimuthOfTile(tile_current);
      cos_az += TMath::Cos(az_tmp);
      sin_az += TMath::Sin(az_tmp);
      tile_count++;
    };
  };

  cos_az /= (Double_t)tile_count;
  sin_az /= (Double_t)tile_count;

  return TMath::ATan2(sin_az,cos_az);
};
  

// draw BBC tile and PMT maps
void BBCtiles::DrawBBC()
{
  TCanvas * bbc_canv = new TCanvas("bbc_canv","bbc_canv",1000,500);
  bbc_canv->Divide(2,1);
  for(int cc=1; cc<=2; cc++) bbc_canv->GetPad(cc)->SetGrid(1,1);
  bbc_canv->cd(1);
  tile_poly[kL]->Draw("text");
  tile_poly[kS]->Draw("textsame");
  bbc_canv->cd(2);
  pmt_poly[kL]->Draw("text");
  pmt_poly[kS]->Draw("textsame");

  for(int cc=1; cc<=2; cc++)
  {
    bbc_canv->cd(cc);
    for(sl=0; sl<2; sl++)
    {
      for(tt=0; tt<18; tt++)
      {
        for(int xx=0; xx<6; xx++)
        {
          hexline[xx][sl][tt]->Draw();
        };
      };
    };
  };
};


// print BBC tile and PMT maps as well as coordinates
void BBCtiles::PrintBBC()
{
  printf("tile\tPMT\tazimuth(d)\tazimuth(r)\tbin\tsl\ttiles_of_pmt\t<pmt_azimuth>\n");
  printf("----\t---\t----------\t----------\t---\t--\t------------\t-------------\n");
  for(tt=1; tt<=36; tt++)
  {
    printf("%4d\t%3d\t%9.2f\t%9.2f\t%3d\t%2d\t%3d,%3d,%3d\t%13.2f\n",
      tt,
      GetPMTOfTile(tt),
      GetAzimuthOfTile(tt,1),
      GetAzimuthOfTile(tt),
      GetBinOfTile(tt),
      GetSlOfTile(tt),
      GetTileOfPMT(GetPMTOfTile(tt),0),
      GetTileOfPMT(GetPMTOfTile(tt),1),
      GetTileOfPMT(GetPMTOfTile(tt),2),
      GetAveAzimuthOfPMT(GetPMTOfTile(tt))
    );
  };
};


// set new event
void BBCtiles::Event(Int_t ew, Int_t sl, Int_t NQT0, Int_t Ind0[16], Int_t ADC0[16], Int_t TAC0[16])
{
  if(ew>=0 && ew<=1 && sl>=0 && sl<=1)
  {
    for(int qq=0; qq<16; qq++)
    {
      Ind[ew][sl][qq]=0;
      ADC[ew][sl][qq]=0;
      TAC[ew][sl][qq]=0;
    };

    NQT[ew][sl] = NQT0;

    for(int qq=0; qq<NQT0; qq++)
    {
      Ind[ew][sl][qq] = Ind0[qq];
      ADC[ew][sl][qq] = ADC0[qq];
      TAC[ew][sl][qq] = TAC0[qq];
    };
  }
  else fprintf(stderr,"ERROR: either ew or sl out of range in BBCtiles::Event\n");
  return;
};
