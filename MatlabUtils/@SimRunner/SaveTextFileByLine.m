function SaveTextFileByLine( obj, filePath, data)
%SAVETEXTFILEBYLINE Summary of this function goes here
%   Detailed explanation goes here

    fid = fopen(filePath,'wt');
    for k=1:length(data)
        fprintf(fid,data{k});
        fprintf(fid,'\n');
    end
    fclose(fid);
end

