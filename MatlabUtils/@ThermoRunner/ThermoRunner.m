classdef ThermoRunner
    %THERMORUNNER This class is used to Initialize the params of a Square
    %Lattice, 1D Truncated Wigner Simulation.
    %   This class creates the folder with a UUID where the params of the
    %   simulation will be stored. In particular, it will attempt to
    %   generate any parameter that has not been specified, it will write
    %   matrices out as files and put their path in the .ini file with
    %   configurations, and will compute the correct number of frames. 
    %   Lastly, this class can then be used to run the c++ simulation
    %   program, assuming it's stored in $HOME/bin.
    
    properties (Access = public)      
        pulseParams = containers.Map('KeyType', 'char', 'ValueType', 'any');
    end
    
    properties (Access = protected)
        managerName = 'TWMCThermoThread';
    end  
    
	methods(Access = protected)
        function res = GenerateSimulationName( obj )
            res = ['TWMC_Thermo_'];
        end
        
        function name = GetManagerName(obj)
            name = obj.managerName;
        end
        
        function PreProcessParams(obj)
            
            if ~isKey(obj.params, 'F')
                fprintf('Assuming a time-dep pulse described');
                pulseData = obj.CreateTimeDependentPulse();
                pulseText = obj.Pulse2CellText(pulseData);
                obj.SaveTextFileByLine(fullfile(obj.simPath, ...
                                        obj.pulseFileName), pulseText);
                obj.params('F_t') = obj.pulseFileName;
            
                if ~isKey(obj.params, 't_end')
                    obj.params('t_end') = obj.params('t_start') + pulseData.times(end);
                end
            end
        end            
    end
end

