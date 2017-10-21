function paramsText = Params2CellText( obj )
%CREATEINIFILE Summary of this function goes here
%   Detailed explanation goes here
    
    paramsText = cell(1,length(obj.params));
    
    allKeys = keys(obj.params);
    for i=1:length(obj.params)
        tmp = obj.params(allKeys{i});
        if isnumeric(tmp)
            if imag(tmp) == 0
                tmp = num2str(tmp);
            else % How to write complex numbers
                tmp = ['( ', num2str(real(tmp)), ', ',...
                            num2str(imag(tmp)), ' )'];
            end
        end
        paramsText{i} = [allKeys{i}, ' = ', tmp];
    end
    
end

