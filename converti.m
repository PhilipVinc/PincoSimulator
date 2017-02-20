fnames = dir('*.dat');
nFiles = length(fnames);

file = importdata( fnames(1).name );
times = size(file,2);
nx = size(file, 1)/2;

beta_tot = zeros(nx,times,nFiles);

for K = 1:nFiles
    file = importdata( fnames(K).name );
    display(fnames(K).name);
    nx = size(file, 1)/2;

    b=reshape(file,times,2,nx);
    c=permute(b,[3,1,2]);
    d=complex(c(:,:,1),c(:,:,2));
    beta_tot(:,:,K)=d;
end

save('_data.mat', 'beta_tot');
exit