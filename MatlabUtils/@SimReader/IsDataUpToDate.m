function isUpdated = IsDataUpToDate( obj, analizedMatPath, dataPath)
%ISDATAUPTODATE Summary of this function goes here
%   Detailed explanation goes here

    isUpdated = false;
    if ~(exist(analizedMatPath, 'file') == 2)
        return
    end

    objToLoad = dir(analizedMatPath);
    dateObj = datetime(objToLoad(1).date, 'Locale',  get(0, 'Language'));
    
    if (exist(dataPath, 'file') == 7)
        % get the Folder Name
        folderName =  dir(dataPath);
        folderName = folderName(1).folder;
        folderName = regexp(folderName,'/','split');
        folderName = folderName{end};

        folderToLoad = dir(fullfile(dataPath, '..'));

        for i=1:length(folderToLoad)
            if strcmp(folderToLoad(i).name, folderName)
                dateFolder = datetime(folderToLoad(i).date, 'Locale',  get(0, 'Language'));
                break;
            end
        end
    else
        folderToLoad = dir(dataPath);
        dateFolder = datetime(folderToLoad(1).date, 'Locale',  get(0, 'Language'));
    end
    
    if dateObj > dateFolder 
        isUpdated = true;
    end
end

