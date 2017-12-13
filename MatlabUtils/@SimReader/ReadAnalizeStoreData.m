function output = ReadAnalizeStoreData( obj )
%READANALIZESTOREDATA Reads and Analizes data, after the path has been correctly 
%set up in the class instance.
%   Detailed explanation goes here
    
    dataPath = fullfile(obj.simPath, obj.simDataFolderName);
    anPath = obj.averagedPath;
    aveDataPath = fullfile(anPath, obj.fileAveragedFileName);
    quanDataPath = fullfile(anPath, obj.fileQuantitiesFileName);
    
    if (obj.IsDataUpToDate(aveDataPath, dataPath) && ~obj.keepInMemory)
        fprintf(['Averaged and Quantities data is up', ...
            ' to date. Loading .mat file.\n']);
       aveData = load(aveDataPath);
       obj.ave = aveData.averaged;
       obj.params = aveData.params;
       clear aveData;
       quanData = load(quanDataPath);
       obj.quan = quanData.quantities;
       clear quanData;
       
       output = 'upToDate mat';
       return
    end
    if ~obj.keepInMemory
        fprintf(['Analized data is not up to date. Reloading.\n']);
    else
        fprintf(['Storing all Trajectories, therefore Reloading all data.\n']);
    end
    
    obj.ReadIniFile();
    obj.ReadRegisterFast();
    obj.ReadDatFiles();
    
    obj.chunkData = cell(1, length(obj.chunkNumbers));
    
    % Check if I need to create the folder for the output data
    if ~exist(fullfile(obj.averagedPath, obj.simDataFolderName))
        mkdir(fullfile(obj.averagedPath, obj.simDataFolderName))
    end
    
    % Iterate through all chunk numbers
    for i=1:length(obj.chunkNumbers)
        
        chunkId = obj.chunkNumbers(i);
        cIPath = obj.ChunkIndexPath(chunkId);
        aveCnkPath = obj.AveragedChunkPath(chunkId);
        
        % Check if it is up to date
        if (obj.IsDataUpToDate(aveCnkPath, cIPath) && ~obj.keepInMemory)
            continue;
        end
        
        % Else reload this chunk
        fprintf( ['Reloading chunk # ', num2str(chunkId), ' :\n']);
        fprintf( ['\tReading...']);
        obj.ReadAllChunk(i);
        fprintf( ['\t Done!\n\tAnalizing...']);
        res = obj.AverageExtractData(obj.chunkData{i}, obj.params);
        fprintf( ['\t Done!\n\tSaving Data...']);
        obj.params = res.params;
        params = obj.params;
       
        % params.n_traj = kn
        
        averaged = res.ave;
        quantities = res.quan;
        save(aveCnkPath, 'averaged', 'params');
        quanCnkPath = obj.QuantitiesChunkPath(chunkId);
        save(quanCnkPath, 'params', 'quantities');
        fprintf( ['\t Done!\n']);
        
        clear params; clear averaged; clear quantities;
        if ~obj.keepInMemory
            obj.chunkData{i} = 0;
        end
    end
    
    obj.AverageMergeChunks();
    obj.QuantitiesMergeChunks();
    
    if obj.keepInMemory
        obj.MergeChunksData();
    end
    
    output = 'good';
    
end

