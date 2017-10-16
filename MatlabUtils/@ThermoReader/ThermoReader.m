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
    end
    
    methods(Access = protected)
        data = AverageExtractData( obj, data, params ); 
    end
    
end

