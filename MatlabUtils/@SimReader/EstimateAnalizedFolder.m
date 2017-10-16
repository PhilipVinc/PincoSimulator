function output = EstimateAnalizedFolder( obj )
%ESTIMATEANALIZEDFOLDER Summary of this function goes here
%   Detailed explanation goes here
    
    pathData = dir(obj.simPath);
    folderPath = pathData(1).folder;
    folders = regexp(folderPath,'/','split');
    
    folderIndex = -1;
    for i=1:min(length(folders), 4)
        curFolder = folders{i};
        
        % If it's one of the good folder structures
        if any(strcmpi(obj.folderStructureSimulationData, curFolder))
            folderIndex = i;
        end
    end
    
    if folderIndex == -1
        obj.averagedPath = fullfile(folderPath, obj.folderStructureAveragedData);
    else
        folders{folderIndex} = obj.folderStructureAveragedData;
        obj.averagedPath = strjoin(folders, '/');
        
    end
    output = obj.averagedPath;
end

