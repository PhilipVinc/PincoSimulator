classdef LiebRunner < BaseSimRunner
    %LIEBRUNNER This class is used to Initialize the params of a Lieb
    %Lattice, 1D Truncated Wigner Simulation.
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
    end
    
    methods(Access = protected)
        function res = GenerateSimulationName( obj )
            res = ['TWLieb_'];
        end
        
        function name = GetManagerName(obj)
            name = obj.managerName;
        end
        
        function UniformLiebParam(obj, key)
            if (isKey(obj.params, key))
                names = {[key,'_A'], [key,'_B'], [key,'_C']};
                for i=1:length(names)
                    nn=names{i};
                    if ~isKey(obj.params, nn)
                        obj.params(nn) = obj.params(key);
                    end
                end
                remove(obj.params, key);
            end
        end

        function MergeCellsInSingleEntry(obj, key)

            notComputed = true;

            names = {[key,'_A'], [key,'_B'], [key,'_C']};
            vals = cell(3,1);
            for i=1:length(names)
                nn=names{i};
                if isKey(obj.params, nn)
                    notComputed = false;
                    tmp = obj.params(nn);
                    remove(obj.params, nn);

                    if (~isvector(tmp) || length(tmp) ~= obj.params('nx')*obj.params('ny') )
                        tmp2 = tmp;
                        tmp = zeros(obj.params('nx')*obj.params('ny'),1);
                        tmp(:) = tmp2;
                    end
                    vals{i} = tmp; 
                end
            end

            % if did nothing then do not edit anything
            if notComputed
                return;
            end

            % Merge
            res = zeros(obj.params('nx')*obj.params('ny')*3,1);
            resA = vals{1}; resB = vals{2}; resC = vals{3};
            for i=1:obj.params('nx')*obj.params('ny')
                res((i-1)*3+1) = resA(i);
                res((i-1)*3+2) = resB(i);
                res(i*3) = resC(i);
            end
            
            obj.params(key) = res;
        end

        function PreProcessParams(obj)
            obj.params('lattice') = 'lieb';
            obj.programName = 'simNew';

            obj.UniformLiebParam('U');
            obj.UniformLiebParam('E');
            obj.UniformLiebParam('F');

            obj.MergeCellsInSingleEntry('U');
            obj.MergeCellsInSingleEntry('E');
            obj.MergeCellsInSingleEntry('F');

            obj.CreateTimeDependent('F');
        end
        
        
        function PostProcessParams(obj)
            a=1+1;
        end
        
    end
    
    methods(Access=public)

        function SetKandGaussian(obj, key)
            if (isKey(obj.params, [key, '_FWHM']) && isKey(obj.params, [key, '_center']))
                Lx=obj.params('nx')*obj.params('ny')*2;
                center = obj.params([key, '_center']);
                maxF = obj.params([key, '_max']);
                FWHM = obj.params([key, '_FWHM']);

                if center > Lx
                    fprintf('ERROR In center of gaussian: too big\n')
                else
                    pts=exp(-(((1:Lx)-center).^2)./(2*FWHM.^2))*maxF^2;
                    obj.params([key,'_A']) = sqrt(pts(1:2:end));
                    obj.params([key,'_B']) = sqrt(pts(1:2:end));
                    obj.params([key,'_C']) = sqrt(pts(2:2:end));
                end
                
                if isKey(obj.params, key)
                    remove(obj.params, key);
                end
            end

            % SetPumpK
            if isKey(obj.params, [key, '_k'])
                obj.UniformLiebParam(key);           
                kval = obj.params([key,'_k']); % Store it in the params.
                
                Lx=obj.params('nx')*obj.params('ny');
                
                xi= (0:(Lx*2-1))/2;
                Fa=obj.params([key,'_A']);
                Fa = Fa.*exp(1j*2*xi(1:2:end)*kval);
                obj.params([key,'_A'])=Fa;
                
                Fb=obj.params([key,'_B']);
                Fb = Fb.*exp(1j*2*xi(1:2:end)*kval);
                obj.params([key,'_B'])=Fb;

                Fc=obj.params([key,'_C']);
                Fc = Fc.*exp(1j*2*xi(2:2:end)*kval);
                obj.params([key,'_C'])=Fc;
            end

            obj.MergeCellsInSingleEntry(key);
        end

        function SetPumpGaussian(obj, FWHM, center, maxF)
            %SETPUMPGAUSSIAN Center:even->C, odd->B,A
            Lx=obj.params('nx')*obj.params('ny')*2;
            if center > Lx
                fprintf('ERROR In center of gaussian: too big\n')
            else
                pts=exp(-(((1:Lx)-center).^2)./(2*FWHM.^2))*maxF^2;
                obj.params('F_A') = sqrt(pts(1:2:end));
                obj.params('F_B') = sqrt(pts(1:2:end));
                obj.params('F_C') = sqrt(pts(2:2:end));
            end
            
            if isKey(obj.params, 'F')
                remove(obj.params, 'F');
            end
        end
        
        function SetPumpK(obj, kval)
            obj.UniformLiebParam('F');           
            obj.params('F_k') = kval; % Store it in the params.
            
            Lx=obj.params('nx')*obj.params('ny');
            
            xi= (0:(Lx*2-1))/2;
            Fa=obj.params('F_A');
            Fa = Fa.*exp(1j*2*xi(1:2:end)*kval);
            obj.params('F_A')=Fa;
            
            Fb=obj.params('F_B');
            Fb = Fb.*exp(1j*2*xi(1:2:end)*kval);
            obj.params('F_B')=Fb;

            Fc=obj.params('F_C');
            Fc = Fc.*exp(1j*2*xi(2:2:end)*kval);
            obj.params('F_C')=Fc;
        end

        function CreateTimeDependent(obj, key)
            % if no speed specified, then no time dependence
            obj.MergeCellsInSingleEntry([key, '_speed']);
            if ~isKey(obj.params, [key,'_speed'])
                return;
            end

            nxy = obj.params('nx')*obj.params('ny');
            
            times = [];
            values = cell(1, 1);
            
            t0 = 0;
            if isKey(obj.params,'t_start')
                t0 = obj.params('t_start');
            end
            
            if isKey(obj.params,[key,'_initialRelaxation'])
                t0 = obj.params([key,'_initialRelaxation']);
            end
            times = [times, t0];
            
            F0 = 0;
            obj.SetKandGaussian([key, '_start']);
            if isKey(obj.params,[key,'_start'])
                F0 = obj.params([key,'_start']);
                remove(obj.params, [key,'_start']);
            else
                F0 = zeros(1, nxy);
            end
                       
            
            speed = obj.params([key,'_speed']);

            obj.SetKandGaussian([key, '_end']);
            Fend =  obj.params([key, '_end']);
            remove(obj.params, [key,'_end']);
            
            if size(F0,1) > size(F0, 2)
                F0 = F0';
            end
            if size(Fend,1) > size(Fend,2)
                Fend=Fend';
            end
            
            if (length(F0) ~= length(Fend))
                if length(F0) < length(Fend) && length(F0)==1
                    tmp = F0;
                    F0 = Fend; F0(:) = tmp;
                else
                    tmp = Fend;
                    Fend = F0; Fend(:) = tmp;
                end
            end
            values{1} = F0;
            
            durations = abs(max(Fend)- max(F0))./speed;
            tups = t0 + durations;
            tEndUp = max(tups);
            
            for t=unique(sort(tups))
                times = [times, t];
                fVals = Fend
                values{end+1} = fVals;
            end
            
            t1 = 0;
            if isKey(obj.params,[key,'_middleRelaxation'])
                t1 = obj.params([key,'_middleRelaxation']);
            end
            
            t1 = t1 + tEndUp;
            times = [times, t1];
            values{end+1} = values{end};
            
            % Descent
            tdowns = t1+durations;
            Fup = values{end};
            for t=unique(sort(tdowns))
                times = [times, t];
                fVals = F0
                fVals(fVals < F0) = F0(fVals < F0); % Fix 
                values{end+1} = fVals;
            end
            
            times(end+1) = floor(times(end)+1);
            values{end+1} = values{end};
            
            pulseData.times = times;
            pulseData.values = values;
            pulseData.fileName = [key,'_t.dat'];

            pulseText = obj.Pulse2CellText(pulseData);
            obj.SaveTextFileByLine(fullfile(obj.simPath, pulseData.fileName), pulseText);
            obj.params('F_t') = pulseData.fileName;
            
            if ~isKey(obj.params, 't_end')
                obj.params('t_end') = obj.params('t_start') + pulseData.times(end);
            end
        end


    end
end

