function [] = PlotCrooks( obj )
%PlotCrooks Plots the Work distributions and croocks linear fit.
%   Arg:
%    - ThermoSimulation Object

%% Init

    nCavs = obj.params.nx*obj.params.ny;
    
    dat = obj.crooksData;
    gaussEqn = 'a*exp(-((x-b)/c)^2)';
    
    clf;
%% Code
    Fss = 16;
    Fsdiff = 2;
    
    cavNames = cell(1, nCavs);
    for i=1:nCavs
        cavNames{i} = num2str(i);
    end
        
    [ha, pos] = tight_subplot(1,2,[.01 .02],[.78 .02],[.05 .05]); 
    axes(ha(1)); ha1 = ha(1);
    
    plot(obj.params.t, obj.params.F_t);
    xlabel('time');
    ylabel('Drive Strength F(t)');
    legend(cavNames);

    axes(ha(2)); ha2 = ha(2);
    
    plot(obj.params.F_t, obj.ave.n_t);
    xlabel('time');
    ha2.YAxisLocation = 'right'
    ylabel('Population n(t)');
    legend(cavNames, 'Location', 'northwest');

    
    
    if (nCavs == 1)
        [ha, pos] = tight_subplot(2,1,[.1 .1],[.05 .285],[.05 .05]);
        di=1;
    else
        [ha, pos] = tight_subplot(2,2,[.1 .1],[.05 .285],[.05 .05]);   
        di = 2;
    end
    
    % Find max prob value
    
    maxP = max(max([dat.P_Wup, dat.P_Wdown]));
    
    for i=1:min(nCavs,2)
        
        % Plot the histograms;
        axes(ha(i))
        bar(dat.W, dat.P_Wup(:, i), 'FaceAlpha', 0.4, 'FaceColor', 'r');
        hold on;
        bar(dat.W, dat.P_Wdown(:, i), 'FaceAlpha', 0.4, 'FaceColor', 'b');
        legend({'P_{A\rightarrow B}(W)'; 'P_{B\leftarrow A}(W)'})
        xlabel('Work W');
        ylabel('Prob P(W)');
        hold off;
        xlim([min(dat.W), max(dat.W)]);
        tmp = ylim; ylim([0, maxP*1.2]); 
        
        % Add vertical lines delimiters
        minWC = min(dat.crooks_W{i}); maxWC = max(dat.crooks_W{i});
        line([minWC, minWC], [0, 1], 'LineWidth',2 , 'Color', 'Black');
        line([maxWC, maxWC], [0, 1], 'LineWidth',2 , 'Color', 'Black');
        
        % Plot the ratios of probabilities
        axes(ha(i+di));
        plot(dat.crooks_W{i}, dat.crooks_logratio{i},'*');
        hold on;
        plot(dat.xFitPts(:,i), dat.yFitPts(:,i), '--', 'LineWidth', 2);
        hold off;
        ll2 = legend({'Data points'; 'Linear Regression'});
        ylabel('$\log\left(\frac{P_{A\rightarrow B}(W)}{P_{B\rightarrow A}(W)}\right)$', 'Interpreter', 'latex');
    
    end
end

