function  output = ReadIniFile(obj)
%READINIFILE Summary of this function goes here
%   Detailed explanation goes here
    
    % Find ini File:
    iniFiles = dir(fullfile(obj.simPath,'*.ini'));
    
    if (~length(iniFiles) == 1)
        output = ['Error: Found', num2str(length(iniFiles)),'ini Files. 1 Expected.'];
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
            
            success = false;
            if exist(dataFilePath, 'file')
                try
                    varData = load(dataFilePath);
                    params.(field) = varData;
                    success = true;
                catch exception
                end
            end
            if ~success
                params.(field) = tmp;
                fprintf(['INI: Could not find file ', dataFilePath, ...
                    ' for key ', field]);
            end
        else
            params.(field) = tmp;
        end
    end
    
    % Check if the local basis size is specified
    if ~isfield(obj.params,'local_cell_size')
        params.local_cell_size=1;
    end
    
    % Store the results
    obj.params = params;

    output = 'success';
end

