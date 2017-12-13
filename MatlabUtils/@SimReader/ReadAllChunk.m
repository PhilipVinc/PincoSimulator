function output = ReadAllChunk( obj, chunkId )
%READALLCHUNK Summary of this function goes here
%   Detailed explanation goes here
    
    % Read the index file
    iF = fopen(obj.ChunkIndexPath(chunkId), 'r');
    entrySize = 1+2*obj.varN;    

    % Open the chunks
    dF = cell(1, obj.varN);
    for i=1:obj.varN
        name = [char(obj.varNames(i)),'_',num2str(obj.chunkNumbers(chunkId)),'.cnk'];
        path = fullfile(obj.simPath, 'data', name);
        opened = false;
        
        % Try to open it with the basic name
        if exist(path, 'file')
            dF{i} = fopen(path, 'r');
            opened = true;
        end

        % Try to open it with the basic name
        if ~opened
            name = ['variable',num2str(i-1),'_',num2str(obj.chunkNumbers(chunkId)),'.cnk'];
            path = fullfile(obj.simPath, 'data', name);
            if exist(path, 'file')
                dF{i} = fopen(path, 'r');
                opened = true;
            end
        end
        
        if ~opened
            name = ['variable',num2str(i),'_',num2str(obj.chunkNumbers(chunkId)),'.cnk'];
            path = fullfile(obj.simPath, 'data', name);
            if exist(path, 'file')
                dF{i} = fopen(path, 'r');
                opened = true;
            else
                fprintf('ERROR: COULD NOT FIND FILE!');
            end
        end
    end
        
    % Find the dataset Size
    fseek(iF, 0, 1);
    endP = ftell(iF);
    fseek(iF, 0, -1);
    entryData = fread(iF, entrySize, 'uint64');
    fseek(iF, 0, -1);

    nTrajs = endP/(8*entrySize);
    trajLengths = entryData(2:2:obj.varN*2);
    % Create the datasets;
    data = cell(1, obj.varN);
    nxy = zeros(1, obj.varN);
    for i=1:obj.varN
        nxy(i) = prod(obj.varDimensions{1});
        if nxy(i) == 0
            nxy(i)=1;
        end
        data{i} = zeros(nxy(i), trajLengths(i), nTrajs);
    end
    
    kn=0;
    while (true)
        % Read the entry in the chunk index
        entryData = fread(iF, entrySize, 'uint64');
        if isempty(entryData)
            break;
        end 
        kn = kn+1;
        % Read the datasets
        for i=1:obj.varN
            % Read the file
            dataSize = fread(dF{i}, 1, 'uint64');
            trajData = fread(dF{i}, dataSize/8, 'float64');
            
            % convert to complex if needed;
            if (obj.varTypes(i) == 22)
                trajData = trajData(1:2:end) + 1i*trajData(2:2:end);
            end
            
            k_traj = reshape(trajData, [nxy(i), trajLengths(i)]);
            data{i}(:,:,kn) = k_traj;
        end
        
    end            

    
    for i=1:obj.varN
        fclose(dF{i});
    end
    fclose(iF);
    
    obj.params.n_traj = kn;
    obj.chunkData{chunkId} = data;
    output = 'ok';
end

