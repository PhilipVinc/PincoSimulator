function res = AverageExtractData( obj, data, params )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

    res.params = params;

    trajId = obj.GetVarId('traj');
    workId = obj.GetVarId('Work');
    areaId = obj.GetVarId('Area');
    
    nt = data{trajId}.*conj(data{trajId}) -1/2;
    dAreat = data{areaId};
    Wt = cumsum(data{workId},2);
    
    res.ave.n_t = mean(nt, 3)';
    res.ave.sigma_n_t = std(nt, 0, 3)';

    res.ave.dA_t = mean(dAreat, 3)';
    res.ave.sigma_dA_t  = std(dAreat, 0, 3)';

    res.ave.A_t = cumsum(res.ave.dA_t);
    res.ave.sigma_A_t  = std(cumsum(data{areaId}, 3), 0,3)';
    
    res.ave.dW_t = mean(data{workId}, 3)';
    res.ave.sigma_dW_t = std(data{workId},0, 3)';
    
    res.ave.W_t = cumsum(res.ave.dW_t);
    res.ave.sigma_W_t = std(cumsum(data{workId}, 3), 0,3)';
    
    nSites = obj.params.nx*obj.params.ny;
    nTraj = size(data{workId},3);
    
    res.quan.WStart = zeros(nSites,nTraj);
    res.quan.WMid = zeros(nSites,nTraj);
    res.quan.WEnd = zeros(nSites,nTraj);
    
    res.params.pulseBegin = zeros(nSites,1);
    res.params.pulseMid = zeros(nSites,1);
    res.params.pulseEnd = zeros(nSites,1);
    
    % Area
    res.ave.area = zeros(nSites,1);
    res.params.F_speeds = zeros(nSites, 1);
    
    dF_t = diff(obj.params.F_t);
    for i=1:nSites
        ids = find(dF_t(:,i));
        beginPulse=max(ids(1)-1, 1);
        endPulse = min(ids(end)+1, length(obj.params.t));
        midPulse = floor(beginPulse + (endPulse -beginPulse)/2);
        
        res.params.pulseBegin(i) = beginPulse;
        res.params.pulseMid(i) = midPulse;
        res.params.pulseEnd(i) = endPulse;        

        res.quan.WStart(i,:) = Wt(i, beginPulse, :);
        res.quan.WMid(i,:) = Wt(i, midPulse, :);
        res.quan.WEnd(i,:) = Wt(i, endPulse, :);
        
        % Area stuff
        Astart = res.ave.A_t(beginPulse, i);
        Amid = res.ave.A_t(midPulse, i);
        Aend = res.ave.A_t(endPulse, i);
        
        Aup = Amid-Astart;
        Adown = Amid-Aend;
        Atot = Aend-Astart;
        
        res.ave.area(i) = Atot;
        
        % Speed
        res.params.F_speeds(i) = dF_t(ids(3),i)/(res.params.t(ids(4))-res.params.t(ids(4)-1));
    end
    
end

