
defaultParams = containers.Map('KeyType', 'char', 'ValueType', 'any');
defaultParams('nx') = 10;
defaultParams('ny') = 1;
defaultParams('PBC') = 1;

defaultParams('U') = 0.05;
defaultParams('detuning') = 0.0;
defaultParams('E_A') = 0.0;
defaultParams('E_B') = 0.0;
defaultParams('E_C') = 0.0;


defaultParams('J_AB') = 10;
defaultParams('J_BC') = 10;

defaultParams('gamma') = 1.0;
defaultParams('n_traj') = 200;
defaultParams('timestep') = 0.001;
defaultParams('beta_init') = 0.0;
defaultParams('beta_init_sigma') = 0.5;

defaultParams('t_end') = 100;

sim=LiebRunner();
sim.parentPath = 'LiebTest/test2';
sim.params = defaultParams;

%sim.SetPumpGaussian(2.5,20,1);
vv=linspace(0,0,10); vv(5)=3;
sim.params('F') = 3;
sim.SetPumpK(pi/2);
%sim.params('F_C') = vv;
scatter(real(sim.params('F_A')),imag(sim.params('F_A')))
sim.params('F_C')
%sim.params('F_C')=0;
%%
sim.SetNCores(12);
sim.CreateSimData();
sim.Execute()

rr=LiebReader(sim.simPath, 'SaveTrajectories', true)

dd=reshape(rr.ave.n_i_t(:,end), [3,rr.params.nx])'

% Plotting
b=bar(dd);
b(1).FaceColor = 'blue';
b(2).FaceColor = 'green';
b(3).FaceColor = 'red';
names = {'A','B','C'};
legend(names);