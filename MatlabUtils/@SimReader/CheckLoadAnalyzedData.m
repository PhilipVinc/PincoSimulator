function loaded = CheckLoadAnalyzedData( obj )
%CHECKLOADANALYZEDDATA Reads and Analizes data, after the path has been correctly 
%set up in the class instance.
%   Detailed explanation goes here
    
    dataPath = fullfile(obj.simPath, obj.simDataFolderName);
    anPath = obj.averagedPath;
    aveDataPath = fullfile(anPath, obj.fileAveragedFileName);
    quanDataPath = fullfile(anPath, obj.fileQuantitiesFileName);
    
    loaded = false;

    if (obj.IsDataUpToDate(aveDataPath, dataPath) && ~obj.forceAnalysis)
        fprintf(['Averaged and Quantities data is up', ...
            ' to date. Loading .mat file.\n']);
       aveData = load(aveDataPath);
       obj.ave = aveData.averaged;
       obj.params = aveData.params;
       clear aveData;
       quanData = load(quanDataPath);
       obj.quan = quanData.quantities;
       clear quanData;
       
       loaded = true;
    end
end