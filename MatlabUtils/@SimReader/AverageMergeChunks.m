function output = AverageMergeChunks( obj )
%AVERAGEMERGECHUNKS Summary of this function goes here
%   Detailed explanation goes here
    
    nChunks = length(obj.chunkNumbers);
    chunks = cell(1, nChunks);

    fprintf(['Averaging ', num2str(nChunks), ' average - value chunks.']);
    fprintf(['\tReading...']);
        
    for i=1:nChunks
        aveCnkPath = obj.AveragedChunkPath(obj.chunkNumbers(i));       
        tmp = load(aveCnkPath);
        chunks{i} = tmp;
    end
    fprintf(['Done!\n\tAveraging...']);
    
    quanNames = fieldnames(chunks{1}.averaged);
    quanNames(strncmp(quanNames, 'params',6)) = []; 
    params = chunks{1}.params;

    for jj=1:length(quanNames)
        val = quanNames{jj};
        averaged.(val) = 0;
    end
    
    n_traj_tot = 0;
    for jj = 1:nChunks
        n_traj_tot = n_traj_tot + chunks{jj}.params.n_traj;
    end
    
    %Remove the std quantities
    stdQuanNames = quanNames(endsWith(quanNames, '_std'));
    quanNames(endsWith(quanNames, '_std')) = [];


    params.n_traj = n_traj_tot;
    for ii=1:nChunks
        data = chunks{ii}.averaged;
        for jj=1:length(quanNames)
            val = quanNames{jj};
            averaged.(val) = averaged.(val) + data.(val);
        end
    end

    for jj=1:length(quanNames)
        val = quanNames{jj};
        averaged.(val) = averaged.(val) / nChunks;
    end

    
    % Compute the std quantities
    for ii=1:nChunks
        data = chunks{ii}.averaged;
        for jj=1:length(stdQuanNames)
            val = stdQuanNames{jj};
            
            aveSqValname = strrep(val, '_std', '_avesq');
            aveValName = strrep(val, '_std', '_avg');
            if isfield(averaged, aveSqValname)            
                averaged.(val) = sqrt(averaged.(aveSqValname) - (data.(aveValName)).^2);
            else
                fprintf(['did compute std for ', aveSqValname, '\n']);
            end
        end
    end

    for jj=1:length(quanNames)
        val = quanNames{jj};
        averaged.(val) = averaged.(val) / nChunks;
    end

    
    aveDataPath = fullfile(obj.averagedPath, obj.fileAveragedFileName);
    fprintf(['Done!\n\tSaving...']);
    save(aveDataPath,'averaged', 'params');
    fprintf(['Done!\n']);
    
    obj.ave = averaged;
    obj.params = params;
end

