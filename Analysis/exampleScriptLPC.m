clear;clc
% Prev onsets and TK interval...
t = [490; 480; 510; 485];
TkInterval = 500;

% Correction gains...
alphas = [ 
           0,  0,  0, 0; 
          .5,  0,  0, 0; 
          .5,  0,  0, 0; 
          .5,  0,  0, 0
         ];

% Noise... 
Mprev = [0;0;0;0]; %[10;10;10;10];
sigmaM = [0;0;0;0]; %[10; 10; 10; 10];
sigmaT = [0;0;0;0]; %[25;25;25;25];

for i = 1:16
    [tNext(:, i), Mn] = LinearPhaseCorrection(t, TkInterval, Mprev, alphas, sigmaM, sigmaT);
    Mprev = Mn;

    %... and repeat...
    t = tNext(:, i);
end



c = {'k', 'r', 'b', 'g'};
for i = 1:length(t)
    plot(tNext(i, :), i, [c{i} 'd'], 'MarkerSize', 15); hold on; grid on;
end
