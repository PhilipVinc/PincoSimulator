function [ res ] = ConvertInterleavedToComplex(obj, data, condition )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
    if condition
        res = data(1:2:end)+1j*data(2:2:end);
    else
        res = data;
    end
end

