classdef ThermoReader < SimReader
    %THERMOREADER Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(Access = public)
        crooksData;
    end

    properties(Access = protected)
        
    end
    
    methods
        function obj = ThermoReader(simPath)

            % Base constructor
            obj = obj@SimReader(simPath);
            
            % Init
            obj.crooksData = obj.ComputeCrooks();
        end
        
        out = ComputeCrooks( obj, varargin );
        estimatedBinning = EstimateCrooksBinning( obj );    
        CheckBinningConvergence(obj);
    end
    
    methods(Access = protected)
        data = AverageExtractData( obj, data, params ); 
    end
    
    methods(Static)
        function sims = ReadFolder(folderPath, varargin)
            simFolds = dir(fullfile(folderPath, 'TWMC*'));
            sims = cell(1, length(simFolds));

            for i=1:length(simFolds)
                sims{i}=ThermoReader(fullfile(folderPath, simFolds(i).name), varargin{:});
            end
        end
    end
end

