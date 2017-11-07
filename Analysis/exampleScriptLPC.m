clear;clc
% Prev onsets and TK interval...
t(1) = 609664;
t(2) = 625226;
t(3) = 625226;
t(4) = 625429;
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
sigmaM = [2;0;0;0]; %[10; 10; 10; 10];
sigmaT = [2;0;0;0]; %[25;25;25;25];

for i = 1:16
    t(1) = t(1) * .9; % simulate a tempo change from the input tapper 
    [tNext(:, i), Mn] = LinearPhaseCorrection(t, TkInterval, Mprev, alphas, sigmaM, sigmaT);
    Mprev = Mn;

    %... and repeat...
    t = tNext(:, i);
end



c = {'k', 'r', 'b', 'g'};
for i = 1:length(t)
    plot(tNext(i, :), i, [c{i} 'd'], 'MarkerSize', 15); hold on; grid on;
end
