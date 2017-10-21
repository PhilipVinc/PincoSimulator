function CheckBinningConvergence( obj )
%CHECKBINNINGCONVERGENCE Summary of this function goes here
%   Detailed explanation goes here
    
    minBins = sqrt(obj.params.n_traj)/2;
    maxBins = sqrt(obj.params.n_traj)*3;
    binCounts = floor(linspace(minBins, maxBins, 30));
    
    nxy = obj.params.nx* obj.params.ny;
    
    nB = length(binCounts);
    T = zeros(nB,nxy); TErr = zeros(nB,nxy);
    DF = zeros(nB,nxy); DFErr = zeros(nB,nxy);

    
    for i=1:length(binCounts)
        histC = obj.ComputeCrooks(binCounts(i));
        T(i,:) = histC.T;        TErr(i,:) = histC.TErr;
        DF(i,:) = histC.DF;     DFErr(i,:) = histC.DFErr;
    end
    
    nBAll = zeros(nB,nxy);
    for i=1:nxy
        nBAll(:,i) = binCounts;
    end
    
    subplot(2,1,1);
    errorbar(nBAll, T, TErr);
    subplot(2,1,2);
    errorbar(nBAll, DF, DFErr);
    
end

