#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>

#include <RooUnfoldResponse.h>
#include <RooUnfoldBayes.h>

#include <random>
#include <iostream>

void unfoldData(){
    const int nPtBins = 6;

    double ptEdges[nPtBins + 1] = {0, 0.0286, 0.0424, 0.05575, 0.071, 0.0945, 0.20};

    const int nPhiBins = 8;

    double phiEdges[nPhiBins + 1] = {-TMath::Pi(), -3.0 * TMath::Pi() / 4.0, -TMath::Pi() / 2.0, -TMath::Pi() / 4.0, 0.0, TMath::Pi() / 4.0, TMath::Pi() / 2.0, 3.0 * TMath::Pi() / 4.0, TMath::Pi()};

    TFile *asymFile[nPtBins];

    for(int i = 0; i < nPtBins; i++){
        asymFile[i] = TFile::Open(Form("pT_2/Yield_JPsiAsymMu_%d.root",i), "READ");
    }

    TH1D *yieldHisto[nPtBins];
    for(int i = 0; i < nPtBins; i++){
        yieldHisto[i] = (TH1D*)asymFile[i]->Get("yieldDeltaPhiStore");
    }

    TFile *AccXe = TFile::Open("corrections.root", "READ");

    TH1D *accxe[nPhiBins];
    TH1D *distToUnfold_corr[nPhiBins];

    TH1D *distToUnfold[nPhiBins];
    for(int i = 0; i < nPhiBins; i++){
        distToUnfold[i] = new TH1D(Form("distToUnfold_%d",i), Form("distToUnfold_%d",i), nPtBins, ptEdges);
        accxe[i] = (TH1D*)AccXe->Get(Form("asymmetry_accxe%d",i));
    }

    for(int i = 1; i <= nPtBins; i++){
        for(int j = 1; j <= nPhiBins; j++){
            double binPhi = yieldHisto[i-1]->GetBinContent(j);
            double binPhiError = yieldHisto[i-1]->GetBinError(j);

            distToUnfold[j-1]->SetBinContent(i, binPhi);
            distToUnfold[j-1]->SetBinError(i, binPhiError);
        }
    }

    for(int i = 0; i < nPhiBins; i++){
        distToUnfold_corr[i] = (TH1D*)distToUnfold[i]->Clone(Form("distToUnfold_corr_%d",i));
        distToUnfold_corr[i]->Divide(accxe[i]);
    }

    TFile *responseFile = TFile::Open("response_new_v3.root", "READ");

    RooUnfoldResponse *response[nPhiBins];

    for(int i = 0; i < nPhiBins; i++){
        response[i] = (RooUnfoldResponse*)responseFile->Get(Form("response_%d",i));
    }
    
    /*TH1D *asym_pT[nPtBins];
    for(int i = 0; i < nPtBins; i++){
        asym_pT[i] = new TH1D(Form("asym_pT_%d", i), Form("asym_pT_%d", i), nPhiBins, phiEdges);
    }*/


    TFile output("unfolded_data_new_v3_corr_v2.root", "RECREATE");

    for (int iPhi = 0; iPhi < nPhiBins; ++iPhi) {

        RooUnfoldBayes unfold(response[iPhi], distToUnfold_corr[iPhi], 4);

        TH1D *hUnfolded = (TH1D*)unfold.Hunfold();
        TMatrixD hCov = unfold.Eunfold(RooUnfold::kCovariance);

        hUnfolded->SetName(Form("hUnfolded_phi%d", iPhi));

        distToUnfold[iPhi]->Write();
        //hTruth[iPhi]->Write();
        hUnfolded->Write();
        hCov.Write(Form("hCov_%d", iPhi));

        TH2D* hCovPt = new TH2D(Form("hCovPt_phi%d", iPhi),"Unfolded pT covariance;Truth pT bin;Truth pT bin",nPtBins, ptEdges, nPtBins, ptEdges);

        for (int i = 1; i <= nPtBins; ++i) {
            for (int j = 1; j <= nPtBins; ++j) {
            hCovPt->SetBinContent(i, j, hCov(i - 1, j - 1));
            }
        }

        hCovPt->Write(Form("hCovPt_%d", iPhi));


        

    }

    output.Close();

    return;
}
