#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>

#include <RooUnfoldResponse.h>
#include <RooUnfoldBayes.h>

#include <random>
#include <iostream>

void getDepFile(){

    const int nPtBins = 6;

    const int nPhiBins = 8;

    double phiEdges[nPhiBins + 1] = {-TMath::Pi(), -3.0 * TMath::Pi() / 4.0, -TMath::Pi() / 2.0, -TMath::Pi() / 4.0, 0.0, TMath::Pi() / 4.0, TMath::Pi() / 2.0, 3.0 * TMath::Pi() / 4.0, TMath::Pi()};

    TFile *unfoldFile = TFile::Open("unfolded_data.root", "READ");

    TH1D *asym_pT[nPtBins];
    for(int i = 0; i < nPtBins; i++){
        asym_pT[i] = new TH1D(Form("asym_pT_%d", i), Form("asym_pT_%d", i), nPhiBins, phiEdges);
    }

    TH1D *unfolded_phi[nPhiBins];
    for(int i = 0; i < nPhiBins; i++){
        unfolded_phi[i] = (TH1D*)unfoldFile->Get(Form("hUnfolded_phi%d", i));
    }

    for(int i = 0; i < nPhiBins; i++){
        for(int j = 0; j < nPtBins; j++){
            double binContent = unfolded_phi[i]->GetBinContent(j+1); 
            double binError = unfolded_phi[i]->GetBinError(j+1);

            asym_pT[j]->SetBinContent(i+1,binContent);
            asym_pT[j]->SetBinError(i+1,binError);
        }
    }

    TFile output("pT_dep.root", "RECREATE");

    for(int i = 0; i < nPtBins; i++){
        asym_pT[i]->Write();
    }

    output.Close();


    return;
}