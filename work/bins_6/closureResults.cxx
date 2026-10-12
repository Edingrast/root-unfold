#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>

#include <RooUnfoldResponse.h>
#include <RooUnfoldBayes.h>

#include <random>
#include <iostream>

void closureResults(){

    const int nPtBins = 6;
    const int nPhiBins = 8;
    const int nIterations = 7;

    TFile *closure[nIterations];

    for (int i = 0; i < nIterations; i++){
        closure[i] = TFile::Open(Form("closure_%diter.root", i+1), "READ");
    }

    TH1D *hPt[nIterations][nPhiBins];
    for(int i = 0; i < nIterations; i++){
        for(int j = 0; j < nPhiBins; j++){
            hPt[i][j] = (TH1D*)closure[i]->Get(Form("hUnfolded_phi%d", j));
        }
    }

    for(int i = 0; i < nIterations; i++){
        for(int j = 0; j < nPhiBins; j++){
            //calculate average statistical uncertainty and relative difference
            double error_sum = 0;
            double differ = 0;
            for(int k = 1; k <= nPtBins; k++){
                error_sum = error_sum + hPt[i][j]->GetBinError(k)/hPt[i][j]->GetBinContent(k);
                if(i > 0){
                    differ = differ + (hPt[i][j]->GetBinContent(k)-hPt[i-1][j]->GetBinContent(k))/hPt[i-1][j]->GetBinContent(k);
                }
            }

            cout << "Average relative statistical error for number of iterations: " << i+1 << " and phi bin: " << j << " is " << (error_sum/nPtBins)*100 << "%" << endl;

            cout << "Average relative difference between iteration: " << i+1 << " and" << i << " in phi bin: " << j << " is " << (differ/nPtBins)*100 << "%" << endl;

        }
    }


    return;
}