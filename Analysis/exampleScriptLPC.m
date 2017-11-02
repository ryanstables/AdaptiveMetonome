% Prev onsets and TK interval...
t = [490; 480; 510; 485];
TkInterval = 500;

% Correction gains...
alphas = [ 
           0, .2, .2, .3; 
          .2,  0, .2, .2; 
          .1, .1,  0, .1; 
          .1,  0,  0, 0
         ];

% Noise... 
Mprev = [0;0;0;0]; %[10;10;10;10];
sigmaM = [0;0;0;0]; %[10; 10; 10; 10];
sigmaT = [0;0;0;0]; %[25;25;25;25];

[tNext, Mn] = LinearPhaseCorrection(t, TkInterval, Mprev, alphas, sigmaM, sigmaT);

Mprev = Mn;
t = fprintf('%.1f\n', tNext);

%... and repeat...



