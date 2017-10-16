function output = ReadDatFiles( obj )
%READDATFILES Summary of this function goes here
%   Detailed explanation goes here
    
    datFiles = dir(fullfile(obj.simPath, 'data', '_*.dat'));
   	obj.params.additionalData = cell(1, length(datFiles));

    for i=1:length(datFiles)
        fPath = fullfile(datFiles(i).folder, datFiles(i).name);
        
        varName = datFiles(i).name(2:end-4);
        varData = load(fPath);
        
        obj.params.(varName) = varData;
        obj.params.additionalData{i} = varName;
    end
    
    output = 'ok';
end

