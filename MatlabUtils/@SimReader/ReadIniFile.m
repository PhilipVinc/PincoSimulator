function  output = ReadIniFile(obj)
%READINIFILE Summary of this function goes here
%   Detailed explanation goes here
    
    % Find ini File:
    iniFiles = dir(fullfile(obj.simPath,'*.ini'));
    
    if (~length(iniFiles) == 1)
        output = 'Error: Found more than 1 ini File';
        fprintf(output);
        return;
    end
    
    iniFilePath = fullfile(obj.simPath, iniFiles(1).name);
    fprintf(['Loading ini file: ', iniFilePath,'\n']);    
    
    % Read the ini file with ini2struct.m
    params = ini2struct(iniFilePath);
    fields = fieldnames(params);
    
    % Convert string with numbers to numbers; save the rest as strings
    for ii=1:length(fields)
        field = fields{ii};
        tmp = params.(field);
        [val, success] = str2num(tmp);
        if success
            params.(field) = val;
        elseif strcmp(tmp(end-3:end), '.dat')
            dataFilePath = fullfile(obj.simPath, tmp);
            if exist(dataFilePath, 'file')
                varData = load(dataFilePath);
                params.(field) = varData;
            else
                params.(field) = tmp;
                fprintf(['INI: Could not find file ', dataFilePath, ...
                    ' for key ', field]);
            end
        else
            params.(field) = tmp;
        end
    end
    
    % Store the results
    obj.params = params;

    output = 'success';
end
