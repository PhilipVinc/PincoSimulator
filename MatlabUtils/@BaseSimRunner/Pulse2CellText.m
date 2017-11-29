function cellText = Pulse2CellText( obj, pulseData )
%PULSE2CELLTEXT Summary of this function goes here
%   Detailed explanation goes here

    nItems = length(pulseData.times);
    
    cellText = cell(1, nItems*3);
    for i=1:nItems
        cellText{i*3-2} = num2str(pulseData.times(i));
        cellText{i*3-1} = num2str(pulseData.values{i});
        cellText{i*3} = '';
    end
    
end

