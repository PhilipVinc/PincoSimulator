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

    %Remove the std quantities from the averaged ones
    stdQuanNames = quanNames(endsWith(quanNames, '_std'));
    quanNames(endsWith(quanNames, '_std')) = [];

    aveSqValNames = quanNames(endsWith(quanNames, '_avesq'));
    for i=1:length(aveSqValNames)
        val = aveSqValNames{i};
        tmp=strrep(val, '_avesq', '_std');
        aveSqValname{i} = tmp;
    end

    for jj=1:length(quanNames)
        val = quanNames{jj};
        averaged.(val) = 0;
    end
    
    n_traj_tot = 0;
    for jj = 1:nChunks
        n_traj_tot = n_traj_tot + chunks{jj}.params.n_traj;
    end
    

    params.n_traj = n_traj_tot;
    for ii=1:nChunks
        data = chunks{ii}.averaged;
        for jj=1:length(quanNames)
            val = quanNames{jj};
            averaged.(val) = averaged.(val) + data.(val).* ...
                (chunks{ii}.params.n_traj/n_traj_tot);
        end
    end

    % Add the std of different quantities
    for jj=1:length(stdQuanNames)
        val = stdQuanNames{jj};
        valErr = strrep(val, '_std', '_err');

        averaged.(val) = 0;
        for ii=1:nChunks
            data = chunks{ii}.averaged;
            averaged.(val) = averaged.(val) + data.(val).* ...
                (chunks{ii}.params.n_traj/n_traj_tot);
        end
        averaged.(val) = sqrt(averaged.(val));
        averaged.(valErr) = averaged.(val)/sqrt(n_traj_tot);
    end


    
    % Compute the std of quantities for which we got avesq and ave
    for jj=1:length(aveSqValNames)
        val = aveSqValNames{jj};
        val = strrep(val, '_avesq', '_std');
        averaged.(val) = 0;
    end

    for jj=1:length(aveSqValNames)
        val = aveSqValNames{jj};
        val = strrep(val, '_avesq', '_std');
        valErr = strrep(val, '_std', '_err');
        valNormAve = strrep(val, '_std', '_normAve');
        aveSqValname = strrep(val, '_std', '_avesq');
        aveValName = strrep(val, '_std', '_avg');

        if ~isfield(averaged, aveValName)
            aveValName = strrep(val, '_std', '');
        end

        if ~isfield(averaged, valNormAve)
            averaged.(valNormAve) = 1;
        end
        
        if (isfield(averaged, aveSqValname) && isfield(averaged, aveValName))            
            averaged.(val) = sqrt((averaged.(aveSqValname) - (averaged.(aveValName)).^2));
            averaged.(valErr) = averaged.(val)/sqrt(n_traj_tot*averaged.(valNormAve));
        else
            fprintf(['did not compute std for ', aveSqValname, '\n']);
        end % TODO: Must fix for different number of elements!!!
    end

    for jj=1:length(aveSqValNames)
        val = aveSqValNames{jj};
        rmfield(averaged, val);
        rmfield(averaged, strrep(val, '_avesq', '_normAve'));
    end    
    
    aveDataPath = fullfile(obj.averagedPath, obj.fileAveragedFileName);
    fprintf(['Done!\n\tSaving...']);
    save(aveDataPath,'averaged', 'params');
    fprintf(['Done!\n']);
    
    obj.ave = averaged;
    obj.params = params;
end

