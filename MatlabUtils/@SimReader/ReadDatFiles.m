function output = ReadDatFiles( obj )
%READDATFILES Summary of this function goes here
%   Detailed explanation goes here
    
    datFiles = dir(fullfile(obj.simPath, 'data', '_*.dat'));
   	obj.params.additionalData = cell(1, length(datFiles));

    for i=1:length(datFiles)
        fPath = fullfile(datFiles(i).folder, datFiles(i).name);
        
        varName = datFiles(i).name(2:end-4);
        varData = load(fPath);
        
        % check if is complex data
        fid=fopen(fPath, 'r');
        line1=fgetl(fid);
        fclose(fid);

        substrs = strsplit(line1,'\t');
        if ((substrs{1}=='%' || substrs{1} == '#') && (length(substrs) > 1))
            if (strcmp(substrs{2}, 'format:complex') && mod(size(varData,2),2)==0)
                varData=varData(:,1:2:end)+1j*varData(:,2:2:end);
            end
        end

        obj.params.(varName) = varData;
        obj.params.additionalData{i} = varName;
    end
    
    output = 'ok';
end

