function pulseData = CreateTimeDependentPulse( obj)
%CREATETIMEDEPENDENTPULSE Summary of this function goes here
%   Detailed explanation goes here

    allKeys = keys(obj.pulseParams);
    nKeys = length(allKeys)
    nxy = obj.params('nx')*obj.params('ny');
    
    times = [];
    values = cell(1, 1);
    
    t0 = 0;
    if isKey(obj.params,'t_start')
        t0 = obj.params('t_start');
    end
    
    if isKey(obj.pulseParams,'initialRelaxation')
        t0 = obj.pulseParams('initialRelaxation');
    end
    times = [times, t0];
    
    F0 = 0;
    if isKey(obj.pulseParams,'value_start')
        F0 = obj.pulseParams('value_start');
    else
        F0 = zeros(1, nxy);
    end
    values{1} = F0;
    
    speed = obj.pulseParams('speed');
    Fend =  obj.pulseParams('value_end');
    durations = (Fend - F0)./speed;
    tups = t0 + durations;
    tEndUp = max(tups);
    
    for t=unique(sort(tups))
        times = [times, t];
        fVals = F0 + speed*(t-t0);
        fVals(fVals > Fend) = Fend(fVals > Fend); % Fix 
        values{end+1} = fVals;
    end
    
    t1 = 0;
    if isKey(obj.pulseParams,'middleRelaxation')
        t1 = obj.pulseParams('middleRelaxation');
    end
    
    t1 = t1 + tEndUp;
    times = [times, t1];
    values{end+1} = values{end};
    
    % Descent
    tdowns = t1+durations;
    Fup = values{end};
    for t=unique(sort(tdowns))
        times = [times, t];
        fVals = Fup - speed*(t-t1);
        fVals(fVals < F0) = F0(fVals < F0); % Fix 
        values{end+1} = fVals;
    end
    
    times(end+1) = floor(times(end)+1);
    values{end+1} = values{end};
    
    pulseData.times = times;
    pulseData.values = values;
end

