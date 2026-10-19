#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>

#include <RooUnfoldResponse.h>

#include <random>


void buildResponse()
{
    TFile *MC_file = TFile::Open("kCohJpsiToMu_new.root", "READ");
    if (!MC_file || MC_file->IsZombie()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    TTree *MC_Tree  = (TTree*) MC_file->Get("DF_2336960461639424/O2treematched");

    if(!MC_Tree) {
        std::cerr << "Error finding MC tree!" << std::endl;
    }

    TFile *DT_weights = TFile::Open("corrections.root", "READ");

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


    const int nPtBins = 6;

    double ptEdges[nPtBins + 1] = {0, 0.0286, 0.0424, 0.05575, 0.071, 0.0945, 0.20};
    const int nPhiBins = 8;

    double phiEdges[nPhiBins + 1] = {-TMath::Pi(), -3.0 * TMath::Pi() / 4.0, -TMath::Pi() / 2.0, -TMath::Pi() / 4.0, 0.0, TMath::Pi() / 4.0, TMath::Pi() / 2.0, 3.0 * TMath::Pi() / 4.0, TMath::Pi()};

    TH1D *hReco[nPhiBins];
    TH1D *hWeights[nPhiBins];
    TH1D *hRecoWeighted[nPhiBins];
    TH1D *hTruth[nPhiBins];
    RooUnfoldResponse *response[nPhiBins];

    for (int iPhi = 0; iPhi < nPhiBins; iPhi++) {

        hReco[iPhi] = new TH1D(Form("hReco_phi%d", iPhi), Form("Reco p_{T}, #phi bin %d", iPhi), nPtBins, ptEdges);
        hWeights[iPhi] = (TH1D*)DT_weights->Get(Form("pT_weights%d",iPhi));
        hTruth[iPhi] = new TH1D(Form("hTruth_phi%d", iPhi), Form("Truth p_{T}, #phi bin %d", iPhi), nPtBins, ptEdges);
        response[iPhi] = new RooUnfoldResponse(hReco[iPhi], hTruth[iPhi], Form("response_%d",iPhi), Form("unfolding response_%d",iPhi));
    }

    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);


    int nEntries = MC_Tree->GetEntries();

    for (int i = 0; i < nEntries; i++) {
        MC_Tree->GetEntry(i);

        // 80% training
        if (uniform(rng) >= 0.80){
            continue;
        }

        if(hasTOF1 == 0 && hasTOF2 == 0){
            continue;
        }

        for (int i = 0; i < nPhiBins; ++i) {
            if (DeltaPhiRandomG >= phiEdges[i] && DeltaPhiRandomG < phiEdges[i + 1]) {
                hTruth[i]->Fill(JpsiPtG);

                int bin = hWeights[i]->FindBin(JpsiPtR);
                double weight = 1.0;

                if (bin >= 1 && bin <= hWeights[i]->GetNbinsX()) {
                    weight = hWeights[i]->GetBinContent(bin);
                }
                response[i]->Fill(JpsiPtR, JpsiPtG, weight);
            }
            if(DeltaPhiRandomR >= phiEdges[i] && DeltaPhiRandomR < phiEdges[i + 1]){
                    hReco[i]->Fill(JpsiPtR);
            }
        }
    }

    for(int i = 0; i < nPhiBins; i++){
        hRecoWeighted[i] = (TH1D*)hReco[i]->Clone(Form("hRecoWeighted_%d",i));
        hRecoWeighted[i]->Multiply(hWeights[i]);
    }

    TFile *output = TFile::Open("response_new_v3.root", "RECREATE");

    for (int i = 0; i < nPhiBins; i++){
      response[i]->Write();
      hTruth[i]->Write();
      hReco[i]->Write();
      hRecoWeighted[i]->Write();
    }

    output->Close();

    return;
}
