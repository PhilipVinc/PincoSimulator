function output = MergeChunksData( obj )
%MERGECHUNKSDATA Summary of this function goes here
%   Detailed explanation goes here

    nChunks = length(obj.chunkNumbers);
    
    % Special case when we only have 1 chunk.
    if nChunks == 1
        obj.data = obj.chunkData{1};
        obj.chunkData{1} = [];
        output = 'Fast';
        return
    end
    
    % Check Empty Chunks
    nEmptyChunks = 0;
    for i=1:nChunks
        if isempty(obj.chunkData{i})
            nEmptyChunks = nEmptyChunks+1;
        end
    end

    if nChunks-nEmptyChunks == 1
        obj.data = obj.chunkData{1};
        obj.chunkData{1} = [];
        output = 'Fast Single';
        return
    end
    
    chunkLenghts = zeros(1, nChunks); 
    for i=1:nChunks
        if isempty(obj.chunkData{i})
            continue;
        end
        chuSiz = size(obj.chunkData{i}{1});
        chunkLenghts(i) = chuSiz(end);
    end
    
    totalLength = sum(chunkLenghts);
    
    
    sizes = cell(1, obj.varN);
    for i=1:obj.varN
        sizes{i} = size(obj.chunkData{1}{i});
        sizes{i}(end) = totalLength;
    end
    
    % copy to new
    obj.data = cell(1, obj.varN);
    for i=1:obj.varN
        obj.data{i} = zeros(sizes{i});
        
        k_t=1;
        for j = 1:nChunks
            if isempty(obj.chunkData{j})
                continue;
            end
            obj.data{i}(:,:,k_t:(k_t+chunkLenghts(j)-1)) = obj.chunkData{j}{i};
            k_t = k_t + chunkLenghts(j);
            
            % Free the memory
            obj.chunkData{j}{i}=0;
        end
    end
    
    output = 'ok';
end

