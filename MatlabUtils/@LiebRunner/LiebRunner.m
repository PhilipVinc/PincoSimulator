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
        managerName = 'TWMCLieb';
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
        
        function PreProcessParams(obj)
            obj.UniformLiebParam('U');
            obj.UniformLiebParam('E');
            obj.UniformLiebParam('F');
        end
        
        
        function PostProcessParams(obj)
            a=1+1;
        end
        
    end
    
    methods(Access=public)
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
 
    end

    
 
end

