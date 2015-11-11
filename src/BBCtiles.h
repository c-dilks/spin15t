#ifndef BBCtiles_
#define BBCtiles_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <map>
#include "TSystem.h"
#include "TMath.h"
#include "TH2Poly.h"
#include "TLine.h"
#include "TCanvas.h"

class BBCtiles : public TObject
{
  public:
    BBCtiles();
    TH2Poly * MakeNewPoly(Int_t sl0);
      
    void HexToCart(Double_t a0, Int_t x0, Int_t y0, Int_t z0, Double_t &xx0, Double_t &yy0);

    Int_t GetBinOfTile(Int_t tile0);
    Int_t GetSlOfTile(Int_t tile0);
    Int_t GetSlOfPMT(Int_t pmt0);
    Int_t GetPMTOfTile(Int_t tile0);
    Int_t GetTileOfPMT(Int_t pmt0, Int_t whichTile);
    Int_t GetXhexOfTile(Int_t tile0);
    Int_t GetYhexOfTile(Int_t tile0);
    Int_t GetZhexOfTile(Int_t tile0);

    Double_t GetAzimuthOfTile(Int_t tile0,Bool_t returnDeg=0);
    Double_t GetAveAzimuthOfPMT(Int_t pmt0);

    Double_t ComputeAzimuthOfTile(Int_t tile0,Bool_t returnDeg=0);
    Double_t ComputeAveAzimuthOfPMT(Int_t pmt0);

    void DrawBBC();
    void PrintBBC();

    void Event(Int_t ew, Int_t sl, Int_t NQT0, Int_t Ind0[16], Int_t ADC0[16], Int_t TAC0[16]);


    TH2Poly * tile_poly[2]; // [sl (sl=0 for small, sl=1 for large)]
    TH2Poly * pmt_poly[2]; // [sl]
    TH2Poly * ev_poly[2][2]; // [sl] [ew]

    
  protected:
    int sl,tt,pp,tpxyz;
    Bool_t initTlines;

    // BBC coordinates map
    // [0=small 1=large] [tile enumerator] [0=tile# 1=pmt# 2=xhex 3=yhex 4=zhex]
    // n.b. TH2Poly bin# = tile# - 18 * sl (where sl=0 for small, 1 for large)
    // - see below for hex coordinates
    Int_t coord[2][18][5];


    /* hexagonal coordinates
     *  
     *      Y
     *     
     *        o 
     *         \     /
     *          \   /
     *           \ /
     *        ----o----o  X
     *           / \
     *          /   \
     *         /     \
     *        o
     *     
     *      Z
     *
     */


    // fast mapping of pmt number to tiles; since there are often more than
    // 1 tile for a given pmt number (up to 3 tiles), we save all tile numbers
    // for each pmt (or zero if nonexistant)
    Int_t coord_pmt[25][3]; // [pmt number] [tile1, tile2 tile3];


    // tile or average pmt azimuths (for fast accessors)
    Double_t azi_tile[37]; // [tile number]
    Double_t azi_pmt[25]; // [pmt number]


    Double_t cellsize[2]; // [sl] // event display hexagon side size
    TLine * hexline[6][2][18]; // [hex side] [sl] [tile]
    Int_t hexlinecolor[2]; // [sl]

    Double_t xcart,ycart; // cartesian coordinates for event display
    Int_t xhex,yhex,zhex; // hex coordinates
    Double_t x[6];
    Double_t y[6];


    Int_t NQT[2][2]; // [ew] [sl]
    Int_t Ind[2][2][16]; // [ew] [sl] [channel]
    Int_t ADC[2][2][16]; // [ew] [sl] [channel]
    Int_t TAC[2][2][16]; // [ew] [sl] [channel]





    ClassDef(BBCtiles,1);

};
#endif
