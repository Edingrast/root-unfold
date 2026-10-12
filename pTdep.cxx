// c++ headers
#include <iostream>
#include <fstream>
using namespace std;

// root headers
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TLorentzVector.h>
#include <TH1F.h>
#include <TMath.h>
#include <TF1.h>
#include <TFormula.h>

Double_t M_min = 2.9; // GeV
Double_t M_max = 3.2; // GeV
Double_t DPhi_max = TMath::Pi();
Double_t DPhi_min = -TMath::Pi();
//Double_t bin_size = (DPhi_max - DPhi_min)/(Double_t)nBins*1000; // MeV
Double_t a = 0.3;

void pTdep(){

  const int nPtBins = 6;

  TFile* file = TFile::Open("pT_dep.root", "READ");

    
  TH1F *asymmetry_pT[nPtBins]; 
  for (Int_t i = 0; i < nPtBins; i++) { 
    asymmetry_pT[i] = (TH1F*)file->Get(Form("asym_pT_%d",i)); 
    if (!asymmetry_pT[i]) {
        std::cerr << "ERROR: Could not find " << "yieldDeltaPhiStore in pT file " << i << std::endl;
        return; 
    }
  }


  TF1 *fits[nPtBins]; 
  for (Int_t iPt = 0; iPt < nPtBins; iPt++){
    fits[iPt] = new TF1( Form("fit_%d", iPt + 1), "[0]*(1+[1]*cos(2*x))", DPhi_min, DPhi_max );
    fits[iPt]->SetParameter(0, 10.0); 
    fits[iPt]->SetParameter(1, 0.01);
    asymmetry_pT[iPt]->Fit(fits[iPt]);
  }



  TH1F* asymmetry_pT_dep = new TH1F("asymmetry_pT_dep", "Azimuthal asymmetry pT dependence", nPtBins, 0, 0.2);
  for (Int_t iPt = 0; iPt < nPtBins; ++iPt){
    asymmetry_pT_dep->SetBinContent(iPt + 1, fits[iPt]->GetParameter(1));
    asymmetry_pT_dep->SetBinError( iPt + 1, fits[iPt]->GetParError(1));
  }

    // Create output ROOT file
  TFile *outputFile = TFile::Open("asym_pT_dep.root", "RECREATE");

    // Write histograms
    for (Int_t i = 0; i < nPtBins; i++) { 
      asymmetry_pT[i]->Write();
    }

    asymmetry_pT_dep->Write();

    // Close files
    outputFile->Close(); 
        
        return;
}
