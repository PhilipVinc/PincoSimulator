classdef SimRunner < handle
    %SIMRUNNER Summary of this class goes here
    %   Detailed explanation goes here
    
    properties (Access = public)
        params = containers.Map('KeyType', 'char', 'ValueType', 'any');
        
        pulseParams = containers.Map('KeyType', 'char', 'ValueType', 'any');
        
        simName = '';
        simPath = '';
        parentPath = '.';
    end
    
    properties (Access = protected)
        programName = 'sim';
        programRelativePath = '$HOME/bin/';
        iniFileName = '_sim.ini';
        pulseFileName = 'F_t.dat'
        nCores = feature('numcores');
    end
    
    methods
        function obj = SimRunner()
            obj.GenerateRandomName();
        end
        
        CreateFolder(obj, folder);
        paramsText = Params2CellText( obj )
        SaveTextFileByLine( obj, filePath, data);
        pulseData = CreateTimeDependentPulse( obj);
        cellText = Pulse2CellText( obj, pulseData );
         
        function SetNCores(obj, nCores)
            obj.nCores = nCores;
        end

        function CreateSimData(obj)
            if ~isKey(obj.params, 't_start')
                obj.params('t_start') = 0;
            end
            
            obj.simPath = fullfile(obj.parentPath, obj.simName);
            obj.CreateFolder(obj.simPath);
            pulseData = obj.CreateTimeDependentPulse();
            
            obj.params('t_end') = obj.params('t_start') + pulseData.times(end);
            if ~isKey(obj.params, 'frames_freq')
                obj.params('n_frames') = floor(obj.params('t_end') - obj.params('t_start'));
            else
                obj.params('n_frames') = floor((obj.params('t_end') - obj.params('t_start'))*obj.params('frames_freq'));
            end
            
            pulseText = obj.Pulse2CellText(pulseData);
            obj.SaveTextFileByLine(fullfile(obj.simPath, ...
                                        obj.pulseFileName), pulseText);
            if isKey(obj.params, 'F')
                remove(obj.params, 'F');
            end
            
            obj.params('F_t') = obj.pulseFileName;
            
            paramsText = obj.Params2CellText();
            obj.SaveTextFileByLine(obj.IniFilePath(), paramsText);
        end
        
        function Execute(obj)
            commandStr = [obj.programRelativePath, obj.programName, ...
                ' -i ', obj.simPath];
            commandStr = [commandStr, ' --processes ', num2str(obj.nCores)];
            tic;
            system(commandStr);
            tt = toc;
            
            fprintf(['ELAPSED TIME: ', num2str(tt), ' s.\n']);
        end
        
        function fName = IniFilePath(obj)
            fName = fullfile(obj.simPath, obj.iniFileName);
        end
        
        function GenerateRandomName(obj)
            obj.simName = ['TWMC_', datestr(now, 'yy-mm-dd_HH-MM-SS')];
        end
    end
    
end

