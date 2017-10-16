classdef TWMCReader < SimReader
    %THERMOREADER Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(Access = public)
        % simData;
    end

    properties(Access = protected)
        
    end
    
    methods
        function obj = TWMCReader(simPath)

            % Base constructor
            obj = obj@SimReader(simPath);
            
            % Init
            % obj.crooksData = obj.ComputeCrooks();
        end
        
        out = ComputeCrooks( obj, varargin );
    end
    
    methods(Access = protected)
        data = AverageExtractData( obj, data, params ); 
    end
    
end

