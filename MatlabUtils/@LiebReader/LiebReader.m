classdef LiebReader < SimReader
    %LIEBREADER Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(Access = public)
        % simData;
    end

    properties(Access = protected)
        
    end
    
    methods
        function obj = LiebReader(simPath, varargin)

            % Base constructor
            obj = obj@SimReader(simPath, varargin{:});
            
        end
    end
    
    methods(Access = protected)
        data = AverageExtractData( obj, data, params ); 
    end
    
    methods(Static)
        function sims = ReadFolder(folderPath)
            simFolds = dir(fullfile(folderPath, 'TWMC*'));
            sims = cell(1, length(simFolds));

            for i=1:length(simFolds)
                sims{i}=TWMCReader(fullfile(folderPath, simFolds(i).name));
            end
        end
    end
    
end

