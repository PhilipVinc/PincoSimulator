function [ a,b,c ] = Lieb1PartCoeffs(Jx,Jy, nx, ny )
%LIEB1PARTCOEFFS Summary of this function goes here
%   Detailed explanation goes here
    
    kx = (0:(nx-1))*pi/nx;
    ky = (0:(ny-1))*pi/ny;
    
    tmp_kx=1+cos(2*kx);
    secx=sec(kx); 
    secx2=secx.*secx;
    sgnsecx = sign(secx);
    cosx=cos(kx);
    
    %correct for exact zero of cos(pi)
    argtmp = Jx^2 * tmp_kx + Jy^2 * (1+cos(2*ky));
    sqrtarg = sqrt(argtmp);
    f = sqrt( argtmp.*secx2 );
    
    if (ny==1 )
        a=zeros(nx,3);
        b=zeros(nx,3);
        c=zeros(nx,3);
        
        a(:,2) = -Jx*sqrt(2)./f;
        b(:,2) = 0;
        c(:,2) = sqrt(2)*Jy*cos(ky)./sqrtarg.*sgnsecx;
        
        a(:,3) = Jy*cos(ky)./sqrtarg.*sgnsecx;
        b(:,3) = sign(cosx)/sqrt(2);
        c(:,3) = Jx ./f;
        
        a(:,1) = a(:,3);
        b(:,1) = -b(:,3);
        c(:,1) = c(:,3);
    else
        a=zeros(kx,ky,3);
        b=zeros(kx,ky,3);
        c=zeros(kx,ky,3);
    end

end

