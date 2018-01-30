classdef TWMCRunner < BaseSimRunner
    %TWMCRUNNER This class is used to Initialize the params of a TWMC, non
    %time-dependent Lattice, 1D Truncated Wigner Simulation.
    %   This class creates the folder with a UUID where the params of the
    %   simulation will be stored. In particular, it will attempt to
    %   generate any parameter that has not been specified, it will write
    %   matrices out as files and put their path in the .ini file with
    %   configurations, and will compute the correct number of frames. 
    %   Lastly, this class can then be used to run the c++ simulation
    %   program, assuming it's stored in $HOME/bin.
    
    properties (Access = public)      
        
    end
    
    properties (Access = protected)
        managerName = 'TWMC';
        pulseFileName = 'F_t.dat'
    end
        
	methods(Access = protected)
        function res = GenerateSimulationName( obj )
            res = ['TWMC_'];
            try
                res = [res, num2str(obj.params('nx')),'x',...
                    num2str(obj.params('ny')),'_'];
            catch
            end
               
        end
        
        function name = GetManagerName(obj)
            name = obj.managerName;
        end
        
        function PreProcessParams(obj)
            % Processing done before standard processing
        end
        
        function PostProcessParams(obj)
            % Processing done after standard processing
        end
    end
    
    methods(Access=public)

    end
end


