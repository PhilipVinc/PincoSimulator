function output = ReadAllChunkNames( obj )
%READALLCHUNKNAMES Summary of this function goes here
%   Detailed explanation goes here

    dataPath = fullfile(obj.simPath, obj.simDataFolderName);
    chunkIndexName = fullfile(dataPath, [obj.simChunkIndexFileName, ...
        '*', obj.simChunkIndexFileTermination]);
    
    preLength = length(obj.simChunkIndexFileName)+1;
    postLength = length(obj.simChunkIndexFileTermination);
    
    chunkFiles = dir(chunkIndexName);
    chunksN = zeros(1,length(chunkFiles));

    for i=1:length(chunkFiles)
        name = chunkFiles(i).name;
        number =str2num(name(preLength:end-postLength));
        chunksN(i) = number;
    end

    fprintf(['Identified ',num2str(length(chunkFiles)) ,' chunks...']);
    
    obj.chunkNumbers = chunksN;
    out='ciao';
end

