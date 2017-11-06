classdef (Abstract) SimReader < handle
    %SIMREADER Summary of this class goes here
    %   Detailed explanation goes here
    
    properties (Access = public)
        params;
        data;
    
        ave;
        quan;
        
        varNames;
        varDimN;
        varDimensions;
    end
    
    properties (Access = protected)
        simPath;
        simDataPath;
        averagedPath;
        
        registerFile;
        magicSignature;
        registerVersion;
        
        additionalTrajDataByteSize;
        additionalTrajDataItems;
        trajDataBegin;
        
        trajData;
        
        chunkNumbers;
        chunksToRead;
        chunkData;

        varTypes;
        varN;

        keepInMemory = false;
    end
    
    properties (Access = protected, Constant=true)
        simDataFolderName = 'data';
        simDatFileName = '_*.dat';
        simRegisterFileName = '_register.bin';
        simChunkIndexFileName = 'index_'; 
        simChunkIndexFileTermination = '.bin';
        
        folderStructureSimulationData = {'simulations'; 
               'simdata'; 'simulationdata'; 'datasets'};
        folderStructureAveragedData = 'averaged';
        folderStructureConvertedData = 'matlab';
        
        fileAveragedFileName = 'averaged.mat';
        fileQuantitiesFileName = 'quantities.mat';
    end
    
    methods(Abstract, Access = protected)
         res = AverageExtractData( obj, data, params );
    end
    
    methods
        function obj = SimReader(simPath)
            % Check that the path exists
            if ~exist(simPath, 'dir')
                errStr = ['ERROR: Folder ', simPath, ' does not exist.'];
                barStr = [UniformString(length(errStr), '-'),'\n'];
                fprintf(barStr);
                fprintf([errStr,'\n']);
                fprintf(barStr);
                return;
            end
            
            obj.simPath = simPath;
            obj.params = [];
            
            obj.magicSignature = [137; 80; 78; 67; 13; 10; 26; 10];
            obj.varNames = strings(1,1);
            
            obj.EstimateAnalizedFolder();
            obj.ReadAnalizeStoreData();
        end
        
        function ReadRegisterFull(obj)
            fprintf(['Reading Register...']);
            obj.ReadSimulationRegister();
            fprintf(['...Traj indexes...']);
            obj.ReadTrajectoriesInRegister();
            fprintf(['Done!\n']);
        end

        function ReadRegisterFast(obj)
            fprintf(['Reading Register...']);
            obj.ReadSimulationRegister();
            fprintf(['...Chunk indexes...']);
            obj.ReadAllChunkNames();
            fprintf(['Done!\n']);
        end

        
        function Read(obj)
            obj.ReadRegister();
            obj.ReadDatFiles();
            
            obj.chunkData = cell(1, length(obj.chunkNumbers));
            for i=1:length(obj.chunkNumbers)
                obj.ReadAllChunk(i);
            end
            
            if length(obj.chunkNumbers) == 1
                obj.MergeChunksData()
            end
        end
                
        % Utilities
        isUpdated = IsDataUpToDate( obj, objPath, folderPath)
        
        function oid = GetVarId(obj, strid)
            indexC = strfind(obj.varNames, strid, 'ForceCellOutput',true);
            oid = find(not(cellfun('isempty', indexC)));
        end
        function dd = Get(obj, strid)
            indexC = strfind(obj.varNames, strid);
            oid = find(not(cellfun('isempty', indexC)));
            dd = obj.data{oid};
        end
        
        output = ReadAllChunkNames( obj );

    end
    
    methods(Access = protected)
        
        output = ReadSimulationRegister(obj);
        output = ReadIniFile(obj);
        output = ReadTrajectoriesInRegister( obj );
        output = ReadAllChunk( obj, chunkId );
        output = MergeChunksData( obj );
        output = ReadDatFiles( obj );
        output = AverageMergeChunks( obj );
        output = QuantitiesMergeChunks( obj );
        
        output = EstimateAnalizedFolder(obj);
        output = ReadAnalizeStoreData( obj );
        
        function cp = ChunkIndexPath(obj, id)
            cp = fullfile(obj.simPath, obj.simDataFolderName, ...
                    [obj.simChunkIndexFileName, ...
                    num2str(obj.chunkNumbers(id)), ...
                    obj.simChunkIndexFileTermination]);
        end
                
        function cp = AveragedChunkPath(obj, id)
            cp = fullfile(obj.averagedPath, obj.simDataFolderName, ...
                    ['averaged_cnk_', ...
                    num2str(obj.chunkNumbers(id)), ...
                    '.mat']);
        end
        
        function cp = QuantitiesChunkPath(obj, id)
            cp = fullfile(obj.averagedPath, obj.simDataFolderName, ...
                    ['quantities_cnk_', ...
                    num2str(obj.chunkNumbers(id)), ...
                    '.mat']);
        end
    end
    
    methods(Access=public)
        function AddTrajectories(obj, ntraj)
            commandStr = ['$HOME/bin/', 'sim', ...
                ' -i ', obj.simPath];
            commandStr = [commandStr, ' --processes ', num2str(2*feature('numcores'))];
            if (ntraj ~= 0)
                commandStr = [commandStr, ' --n_traj ' , num2str(ntraj)];
            end
            fprintf([commandStr, '\n']);
            tic;
            system(commandStr);
            tt = toc;
            
            fprintf(['ELAPSED TIME: ', num2str(tt), ' s.\n']);
        end
        
        function Reload(obj)
            obj.ReadAnalizeStoreData();
        end
        
        function path = GetFolderName(obj)
            path = obj.simPath;
        end
    end
end

