function CreateFolder( obj, folder )
%CREATEFOLDER Summary of this function goes here
%   Detailed explanation goes here
    
    if exist(folder, 'dir')
        
    else
        mkdir(folder);
    end
    

end

