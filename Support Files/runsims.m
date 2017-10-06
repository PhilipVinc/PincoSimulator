%%%%%%%%%%%%%%%%%%%%%%%%
%cd /home/vicentini/portable
runfolder = ' simulations';

system('make');
system(strcat('cp twmc', runfolder, '/twmc'));
cd simulations

vals={'ny = 1';
'U = 0.1';
'J = 0.1';
'gamma = 0.5';
'omega = 1.0';
't_start = 0';
%'t_end = 800';
'n_frames = 400';
'n_traj = 1000';
'processes = 24';
'timestep_factor = 9'};

F_arr = linspace(1, 1.4, 22)';
nx_arr = [2;4;8;16;32];

for i=1:length(vals)
   eval(vals{i});
end

system('pwd')

for i=1:length(nx_arr)
   for j=1:length(F_arr)
       nx = nx_arr(i);
       F = F_arr(j);
       display(nx);
       display(F);
       l_vals = vals;
       l_vals{end+1} = strcat('nx =', ' ' , num2str(nx));
       l_vals{end+1} = strcat('F =', ' ' , num2str(F));

       x0=1.15;
       l=0.1;
       m = 1000;
       N = 3000;

       time = m+(N-m)*exp(-(F-x0)^2/(2*l^2));
       l_vals{end+1} = strcat('t_end =', ' ' , num2str(time));


       clear params
       for K=1:length(l_vals)
           eval(strcat('params.',l_vals{K},';'));
       end

       runName = strcat('TWMC_U_',num2str(params.U),':nx_', ...
           num2str(params.nx),':F_',num2str(params.F));
       iniName = strcat(runName,'.ini');
       fid = fopen(iniName,'wt');
       for k=1:length(l_vals)
           fprintf(fid,l_vals{k});
           fprintf(fid,'\n');
       end
       fclose(fid);

       command = strcat('./twmc -ixxx', iniName, ' -oxxx', runName);
       command = strrep(command,'xxx', ' ');
       system(command);

       cd(runName);
       fnames = dir('computedTraj*.dat');
       nFiles = length(fnames);

       file = importdata( fnames(1).name );
       times = size(file,1);
       nx = size(file, 2)/2;

       % Read data files
       beta_tot = zeros(nx,times,nFiles);
       for K = 1:nFiles
           file = load(fnames(K).name);
           ktraj=file(:,1:2:end)-1i*file(:,2:2:end);
           beta_tot(:,:,K)=ktraj';
       end


       cd('..');
       save(strcat(runName,'.mat'), 'beta_tot', 'params');
       rmdir(runName,'s');
   end
end


%%%%%