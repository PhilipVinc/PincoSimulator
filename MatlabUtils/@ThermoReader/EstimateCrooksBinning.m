function estimatedBinning = EstimateCrooksBinning( obj )
%ESTIMATECROOKSBINNING Summary of this function goes here
%   Detailed explanation goes here

    dataSize = size(obj.quan.WEnd,2);
    estimateBinning = ceil(sqrt(dataSize));

    
%     binCounts = 10:10:400;
%     nB = length(binCounts);
%     T = zeros(nB,2); TErr = zeros(nB,2);
%     DF = zeros(nB,2); DFErr = zeros(nB,2);
% 
%     data = ThermoReader('TestSim3J/');
% 
%     for i=1:length(binCounts)
%         histC = data.ComputeCrooks(binCounts(i));
%         T(i,:) = histC.T;        TErr(i,:) = histC.TErr;
%         DF(i,:) = histC.DF;     DFErr(i,:) = histC.DFErr;
%     end

    estimatedBinning = floor(1.6*estimateBinning);
end

