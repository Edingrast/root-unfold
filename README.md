# root-unfold
Repository for running RooUnfold in Docker. The individual scripts go through the unfolding procedure step-by-step. For Docker install check [here](https://codimd.web.cern.ch/6MP0rgOZQ56H9IeR8_MnJA).

## Dockerfile
File to tell Docker how to build the desired image.

## buildResponse.cxx
Script to create a response matrix. This script is running on matched MC reco and MC true events and saves a response matrix in a root file.
This response is build on 80% of the data sample.
For the purposes of this analysis, the response matrix and unfolding is done in DeltaPhi slices.

## work/bins_6/closureTest.cxx
Script to do the closure test on the remaining 20% of the MC data. It creates a root file with reco, true and unfolded distribution (using iterative bayesian unfolding) and also covariance matrix.
Furthermore, the resulting root file is containing plots to see whether the unfolding is successful. These plots are ratio (unfolded/true) distribution (expected to be 1 within uncertainties) and also relative ratio ((unfolded-true)/true) which is, in my opinion, enough to see whether the closure test works (expected to be 0 within uncertainties).

## work/bins_6/closureResults.cxx
Script to write the result of closure test for comparison. It calculate the average statistical uncertainty in each phi slice and also average relative difference change between the number of iterations. Statistical uncertainty should grow with number of iterations and average relative difference between iterations should be decreasing. We are looking for the sweet spot is below ~1% but the statistical uncertainty is not too high.

## unfoldData.cxx
After determining the correct number of iterations, you can run it on data and get the result.
As input I use data after signal extraction and Accxe correction. I have separate file with DeltaPhi distribution in each pT bin so I have to create a pT distribution in DeltaPhi slices before I unfold it.

## getDepFile.cxx
Get back the DeltaPhi distributions in pT bins.

## pTdep.cxx
Yields the a2 dependence on pT.

## 
