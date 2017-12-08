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
        function sims = ReadFolder(folderPath, varargin)
            
            p=inputParser;
            p.KeepUnmatched = true;
            addRequired(p,'folderPath', @isstr);
            addParameter(p,'SortKey', '', @isstr);
            parse(p, folderPath, varargin{:});
            pars=p.Results;

            
            simFolds = dir(fullfile(pars.folderPath, 'TWLieb_*'));
            sims = cell(1, length(simFolds));

            for i=1:length(simFolds)
                sims{i}=LiebReader(fullfile(pars.folderPath, simFolds(i).name), varargin{:});
            end
            
            if ~strcmp('', pars.SortKey)
                if isfield(sims{1}.params, pars.SortKey)
                    fprintf(['Sorting...']);
                    sVals = zeros(length(sims),1);
                    for i=1:length(simFolds)
                        sVals(i) = sims{i}.params.(pars.SortKey);
                    end
                    
                    [~, sKey] = sort(sVals);
                    sims = sims(sKey);
                    
                    fprintf(['Succesfull!\n']);
                else
                    fprintf(['ERROR: Cannot Sort because ', pars.SortKey,...
                        ' is not a valid field of params.\n']);
                end
            end
        end
    end
    
end

