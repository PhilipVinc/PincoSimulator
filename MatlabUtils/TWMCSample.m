
defaultParams = containers.Map('KeyType', 'char', 'ValueType', 'any');
defaultParams('nx') = 10;
defaultParams('ny') = 2;

defaultParams('U') = 0.1;
defaultParams('omega') = 1.0;

defaultParams('J') = 0.45;

defaultParams('gamma') = 1.0;
defaultParams('n_traj') = 200;
defaultParams('timestep') = 0.001;
defaultParams('beta_init') = 0.0;
defaultParams('beta_init_sigma') = 0.5;
defaultParams('F') = 3;
defaultParams('F_Noise_Type') = 'Gaussian';
defaultParams('F_Noise_Val_0') = 1;

defaultParams('t_end') = 100;

sim=TWMCRunner();
sim.parentPath = 'TestRun/simulations';
sim.params = defaultParams;

%%
sim.SetNCores(12);
sim.CreateSimData();
sim.Execute()

rr=TWMCReader(sim.simPath, 'SaveTrajectories', true)

