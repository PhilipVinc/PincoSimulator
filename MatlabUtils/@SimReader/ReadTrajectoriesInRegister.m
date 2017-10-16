function output = ReadTrajectoriesInRegister( obj )
%READTRAJECTORIESINREGISTER Summary of this function goes here
%   Detailed explanation goes here
    

    fseek(obj.registerFile, 0, 1);
    regEnd = ftell(obj.registerFile);
    trajDataSize = regEnd - obj.trajDataBegin;
    fseek(obj.registerFile, obj.trajDataBegin, -1);    
    
    nTrajsInRegister = trajDataSize/(8*(4+obj.additionalTrajDataItems));
    obj.trajData = cell(nTrajsInRegister,1);
    
    readTrajs = 0;
    
    while(readTrajs < nTrajsInRegister)
        tmp = fread(obj.registerFile, 4, 'uint64');
        
        %if isempty(tmp)
        %    break;
        %end
       
        tmpData = fread(obj.registerFile, obj.additionalTrajDataItems, 'float64');
       
        readTrajs = readTrajs +1;
        
        var.traj_id = tmp(1);
        var.chunk_id =  tmp(2);
        var.chunk_offset =  tmp(3);
        var.continuation_offset =  tmp(4);
        
        obj.trajData{readTrajs} = var;
    end
    
    % Find all chunk numbers
    chunksN = zeros(1,readTrajs);
    for i=1:readTrajs
        chunksN(i) = obj.trajData{i}.chunk_id;
    end
    obj.chunkNumbers = unique(chunksN);
    
    
    output= length(obj.trajData);
end

