function histData = ComputeCrooks( obj, varargin)
%COMPUTECROOKS Summary of this function goes here
%   Detailed explanation goes here
    

    % only want 3 optional inputs at most
    numvarargs = length(varargin);
    if numvarargs > 1
        error('myfuns:somefun2Alt:TooManyInputs', ...
            'requires at most 3 optional inputs');
    end

    % set defaults for optional inputs
    optargs = {-1};

    % now put these defaults into the valuesToUse cell array, 
    % and overwrite the ones specified in varargin.
    optargs(1:numvarargs) = varargin;
    % or ...
    % [optargs{1:numvarargs}] = varargin{:};

    % Place optional args in memorable variable names
    [n_bins] = optargs{:};

    if n_bins == -1
        n_bins = obj.EstimateCrooksBinning();
    end
        
    
    Wup = (obj.quan.WMid-obj.quan.WStart)';
    Wdown = (obj.quan.WMid-obj.quan.WEnd)';
    
    m_Wup = mean(Wup, 1);       m_Wdown = mean(Wdown, 1);
    std_Wup = std(Wup, 1);      std_Wdown = std(Wdown, 1);

    % This is the number of points other after/before the mean values of
    % the two distributions, to be considered in the linear fit.
    additionalPoints = 4;
    additionalPointsLow = 0;
    additionalPointsHigh = 0;

    %Define the histogram bins
    xmin = min(min(cat(1,Wup,Wdown)));
    xmax = max(max(cat(1,Wup,Wdown)));
    xbins = linspace(xmin,xmax,n_bins+1);

    % A gaussian with parameters:
    %   [1]='a' -> Gaussian Area. (Number of trajectories)
    %   [2]='b' -> mean
    %   [3]='c' -> std
    gaussEqn = 'a*exp(-((x-b)/c)^2)';
    nxy = obj.params.nx*obj.params.ny;

    histData.nxy = nxy;
    histData.W = xbins;
    histData.P_Wup = zeros(n_bins, nxy);
    histData.P_Wdown = zeros(n_bins, nxy);
    histData.crooks_W = cell(1, nxy);
    histData.crooks_ratio = cell(1, nxy);
    histData.crooks_logratio = cell(1, nxy);

    histData.crooksFits = cell(1, nxy);
    histData.beta = zeros(1, nxy); histData.betaErr = zeros(1, nxy);
    histData.T = zeros(1, nxy); histData.TErr = zeros(1, nxy);
    histData.DF = zeros(1, nxy); histData.DFErr = zeros(1, nxy);
    histData.DS = zeros(1, nxy); histData.DSErr = zeros(1, nxy);
    
    histData.xFitPts = zeros(100,nxy); histData.yFitPts = zeros(100, nxy);
    
    for i=1:nxy
        % Fit the Wup and Wdown distributions with a gaussian.
        % The starting point of the fit will be given by the std and mean
        % computed 'by hand' on the distribution.
        [binsUp,edgesUp]=histcounts(Wup(:,i),xbins, 'Normalization', 'probability');
        xUp = edgesUp(1:end-1) + diff(edgesUp)./2;
        
        fitUpStart=[max(binsUp), m_Wup(i), std_Wup(i)];
        ffUp=fit(xUp',binsUp', gaussEqn, 'Start', fitUpStart);
        coefsUp = coeffvalues(ffUp);
        confUp = confint(ffUp);

        [binsDown,edgesDown]=histcounts(Wdown(:,i),xbins, 'Normalization', 'probability');
        xDown = edgesDown(1:end-1) + diff(edgesDown)./2;
    
        fitDownStart=[max(binsDown), m_Wdown(i), std_Wdown(i)];
        ffDown=fit(xDown',binsDown', gaussEqn, 'Start', fitDownStart);
        coefsDown = coeffvalues(ffDown);
        confDown = confint(ffDown);
        
        % Give sensible names to the coefficients, and compute the errors.
        normUp = coefsUp(1);    normUpErr = abs(confUp(1,1)-confUp(2,1))/2;
        meanUp = coefsUp(2);    meanUpErr = abs(confUp(1,2)-confUp(2,2))/2;
        sigmaUp = coefsUp(3);   sigmaUpErr = abs(confUp(1,3)-confUp(2,3))/2;
        normDown = coefsDown(1);
        meanDown = coefsDown(2);    
        sigmaDown = coefsDown(3);   
        normDownErr = abs(confDown(1,1)-confDown(2,1))/2;
        meanDownErr = abs(confDown(1,2)-confDown(2,2))/2;
        sigmaDownErr = abs(confDown(1,3)-confDown(2,3))/2;

        % Compute <Wup>-<Wdown>
        updown_dist = meanUp-meanDown;
    
        % Compute the intersection point between the two GAUSSIAN distributions
        % obtained by the fit.
        lg = log(normUp/normDown);
        x_intersec = (meanDown*power(sigmaUp,2) - ...
            sigmaDown*(meanUp*sigmaDown + ...
            sigmaUp*sqrt(power(meanUp - meanDown,2) + (-power(sigmaUp,2) + ...
            power(sigmaDown,2))*lg)))/((sigmaUp -...
                                            sigmaDown)*(sigmaUp + sigmaDown));

        % CrooksPlot stuff (Fitting and other)
        func = @(x) (normUp*exp(-((x-meanUp)./sigmaUp).^2))./ ...
            (normDown*exp(-((x-meanDown)./sigmaDown).^2));

        % Find the bin # for the Mean(Wup) and Mean(Wdown)
        [~, idUp] = min(abs(xbins - meanUp));
        [~, idDown] = min(abs(xbins - meanDown));
        idStart = min([idUp, idDown]);
        idEnd = max([idUp, idDown]);
        
        % Extend the range of the fit by adding some additionalPoints
        %idStart = max(idStart - additionalPoints,length(binsDown) ...
        %                                   -find(fliplr(binsDown)<= 0, 1)+1);
        %idEnd = min(idEnd + additionalPoints, find(binsUp<= 0, 1)-1 );
        idStart = max(idStart - (additionalPointsLow + additionalPoints),1);
        idEnd = min(idEnd + (additionalPointsHigh + additionalPoints),length(binsUp));

        % Define a new array of points, holding X (Work) and Y ((Pup/Pdown)) 
        % that we will fit in logplot.
        crooksXPts = xbins(idStart:idEnd);
        crooksYPts = binsUp(idStart:idEnd)./binsDown(idStart:idEnd);

        % find elements that are not between -100,100;
        rmPts = find(any(log(crooksYPts)>-100&log(crooksYPts)<100,1)==0); 
        % Remove infinities and Nans, points we don't want, so that the code
        % will work all the time.
        crooksXPts(rmPts) = [];
        crooksYPts(rmPts) = [];

        % Perform a linear fit on the points, only if we have 2 or more points,
        % otherwise throw an exception and fill some fake data.
        if (length(crooksXPts) > 1)
            [ffCrooks, ffCrooksErrStruct]=polyfit(crooksXPts,...    
                                                        log(crooksYPts), 1);
            ffCrooksErr = sqrt(diag(inv(ffCrooksErrStruct.R)*...
                                        inv(ffCrooksErrStruct.R')).*...
                                            ffCrooksErrStruct.normr.^2./...
                                                ffCrooksErrStruct.df);
            % Extract the beta and it's error.
            beta = -ffCrooks(1);
            betaErr = ffCrooksErr(1);
            deltaF = ffCrooks(2)/ffCrooks(1);
            deltaFErr = ffCrooksErr(2);
            deltaS = 0; % (deltaU-deltaF)*beta;
            deltaSErr = 0;

            xFitPts = linspace(min(crooksXPts),max(crooksXPts),100);
            yFitPts = polyval(ffCrooks, xFitPts);
            failedFitFlag = false;
        else
            display('error while processing this file!');
            display('Not enough points for fit');
            crooksXPts = [0,1];
            crooksYPts = [0,1];
            beta= 0;
            betaErr =0;
            deltaF=0;
            deltaFErr= 0;
            deltaS=0;
            deltaSErr=0;
            ffCrooks = 0;
            xFitPts = crooksXPts; yFitPts = crooksYPts;
            failedFitFlag = true;
        end
    
        histData.P_Wup(:,i) = binsUp;
        histData.P_Wdown(:,i) = binsDown;
        
        histData.crooks_W{i} = crooksXPts;
        histData.crooks_ratio{i} = crooksYPts;
        histData.crooks_logratio{i} = log(crooksYPts);
        
        histData.crooksFits{i} = ffCrooks;
        histData.xFitPts(:,i) = xFitPts';
        histData.yFitPts(:,i) = yFitPts';
        
        histData.beta(i) = beta; histData.betaErr(i) = betaErr;
        histData.T(i) = 1.0/beta; histData.TErr(i) = betaErr/beta^2;
        histData.DF(i) = deltaF; histData.DFErr(i) = deltaFErr;
        histData.DS(i) = deltaS; histData.DSErr(i) = deltaSErr;
        
    end
    histData.W = xUp';
end


