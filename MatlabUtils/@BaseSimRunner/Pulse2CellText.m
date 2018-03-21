function cellText = Pulse2CellText( obj, pulseData )
%PULSE2CELLTEXT Summary of this function goes here
%   Detailed explanation goes here

    nItems = length(pulseData.times);
    
    cellText = cell(1, nItems*3);
    for i=1:nItems
        cellText{i*3-2} = num2str(pulseData.times(i));
        if any(imag(pulseData.values{i}))
            tmp = reshape([real(pulseData.values{i}); imag(pulseData.values{i})],[1,length(pulseData.values{i})*2])
            cellText{i*3-1} = num2str(tmp);
        else
            cellText{i*3-1} = num2str(pulseData.values{i});
        end
        cellText{i*3} = '';
    end
    
end

