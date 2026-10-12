#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>

#include <RooUnfoldResponse.h>
#include <RooUnfoldBayes.h>

#include <random>
#include <iostream>



void closureTest()
{   
    const int nPtBins = 6;

    double ptEdges[nPtBins + 1] = {0, 0.0286, 0.0424, 0.05575, 0.071, 0.0945, 0.20};

    const int nPhiBins = 8;

    double phiEdges[nPhiBins + 1] = {-TMath::Pi(), -3.0 * TMath::Pi() / 4.0, -TMath::Pi() / 2.0, -TMath::Pi() / 4.0, 0.0, TMath::Pi() / 4.0, TMath::Pi() / 2.0, 3.0 * TMath::Pi() / 4.0, TMath::Pi()};



    TFile *responseFile = TFile::Open("response_phiPt.root", "READ");

    RooUnfoldResponse *response[nPhiBins];

    for(int i = 0; i < nPhiBins; i++){
        response[i] = (RooUnfoldResponse*)responseFile->Get(Form("response_%d",i));
    }

    TFile *MC_file = TFile::Open("kCohJpsiToMu_new.root", "READ");
    if (!MC_file || MC_file->IsZombie()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    TTree *MC_Tree  = (TTree*) MC_file->Get("DF_2336960461639424/O2treematched");

    if(!MC_Tree) {
        std::cerr << "Error finding MC tree!" << std::endl;
    }



   

    double JpsiPtR, JpsiPtG, JpsiMR, JpsiMG, DeltaPhiR, DeltaPhiRandomR, DeltaPhiG, DeltaPhiRandomG;
    bool hasTOF1, hasTOF2;


    MC_Tree->SetName("MC_Tree");
    MC_Tree->SetBranchAddress("fJpsiPtG", &JpsiPtG);
    MC_Tree->SetBranchAddress("fJpsiMG", &JpsiMG);
    MC_Tree->SetBranchAddress("fJpsiPt", &JpsiPtR);
    MC_Tree->SetBranchAddress("fJpsiM", &JpsiMR);
    MC_Tree->SetBranchAddress("fHasTOF1", &hasTOF1);
    MC_Tree->SetBranchAddress("fHasTOF2", &hasTOF2);
    MC_Tree->SetBranchAddress("fJpsiPhiCharge", &DeltaPhiR);
    MC_Tree->SetBranchAddress("fJpsiPhiRandom", &DeltaPhiRandomR);
    MC_Tree->SetBranchAddress("fJpsiPhiChargeG", &DeltaPhiG);
    MC_Tree->SetBranchAddress("fJpsiPhiRandomG", &DeltaPhiRandomG);

    

    TH1D *hReco[nPhiBins];
    TH1D *hTruth[nPhiBins];

    for (int iPhi = 0; iPhi < nPhiBins; iPhi++) {

        hReco[iPhi] = new TH1D(Form("hReco_phi%d", iPhi), Form("Reco p_{T}, #phi bin %d", iPhi), nPtBins, ptEdges);

        hTruth[iPhi] = new TH1D(Form("hTruth_phi%d", iPhi), Form("Truth p_{T}, #phi bin %d", iPhi), nPtBins, ptEdges);
    }


    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);


    Long64_t nEntries = MC_Tree->GetEntries();

    for (Long64_t i = 0; i < nEntries; i++) {

        MC_Tree->GetEntry(i);

        double r = uniform(rng);

        // Skip 80% training events
        if (r < 0.80){
            continue;
        }

        if(hasTOF1 == 0 && hasTOF2 == 0){
            continue;
        }


        int phiBin = -1;

        for (int i = 0; i < nPhiBins; ++i) {
            if (DeltaPhiRandomG >= phiEdges[i] && DeltaPhiRandomG < phiEdges[i + 1]) {
                phiBin = i;
                break;
            }
        }


        hTruth[phiBin]->Fill(JpsiPtG);
        hReco[phiBin]->Fill(JpsiPtR);
    }


    TFile output("closure_6iter_new.root", "RECREATE");

    //TMatrixD *hCov[nPhiBins];

    for (int iPhi = 0; iPhi < nPhiBins; ++iPhi) {

        RooUnfoldBayes unfold(response[iPhi], hReco[iPhi], 6);

        TH1D *hUnfolded = (TH1D*)unfold.Hunfold();
        TMatrixD hCov = unfold.Eunfold(RooUnfold::kCovariance);

        hUnfolded->SetName(Form("hUnfolded_phi%d", iPhi));

        hReco[iPhi]->Write();
        hTruth[iPhi]->Write();
        hUnfolded->Write();
        hCov.Write(Form("hCov_%d", iPhi));

        TH2D* hCovPt = new TH2D(Form("hCovPt_phi%d", iPhi),"Unfolded pT covariance;Truth pT bin;Truth pT bin",nPtBins, ptEdges, nPtBins, ptEdges);

        for (int i = 1; i <= nPtBins; ++i) {
            for (int j = 1; j <= nPtBins; ++j) {
            hCovPt->SetBinContent(i, j, hCov(i - 1, j - 1));
            }
        }

        hCovPt->Write(Form("hCovPt_%d", iPhi));

        hUnfolded->Sumw2();
        hTruth[iPhi]->Sumw2();


        TH1D *hRatio = (TH1D*)hUnfolded->Clone(Form("hRatio_phi%d", iPhi));

        hRatio->Divide(hTruth[iPhi]);

        hRatio->Write();
    }

    output.Close();

    return;
}