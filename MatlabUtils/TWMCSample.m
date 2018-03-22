
defaultParams = containers.Map('KeyType', 'char', 'ValueType', 'any');
defaultParams('nx') = 4;
defaultParams('ny') = 4;

defaultParams('U') = 0.1;
defaultParams('omega') = 1.0;

defaultParams('J') = 0.225;

defaultParams('gamma') = 1.0;
defaultParams('n_traj') = 1000;
defaultParams('timestep') = 0.0005;
defaultParams('beta_init') = 0.0;
defaultParams('beta_init_sigma') = 0.5;
defaultParams('F') = 1.575;
defaultParams('F_Noise_Type') = 'Gaussian';
defaultParams('F_Noise_Val_0') = 1;
defaultParams('PBC') = 'true';

defaultParams('t_end') = 400;


sim=TWMCRunner();
sim.SetProgram('simNew');
sim.parentPath = 'TestRun/simulations';
sim.params = defaultParams;

%%
sim.SetNCores(12);
sim.CreateSimData();
sim.Execute()

rr2=TWMCReader(sim.simPath, 'SaveTrajectories', true)

