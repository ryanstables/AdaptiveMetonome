clear;clc

% Prev onsets and TK interval...
t(1) = 0;
t(2) = 500;
t(3) = 100;
t(4) = 0;
TkInterval = 500;

% Correction gains...
% alphas = [ 
           0,  0,  0, 0; 
          .5,  0,  0, 0; 
          .5,  0,  0, 0; 
          .5,  0,  0, 0
         ];

% alphas = [ 
%            0,  0.25,  0.25, 0.25; 
%            0,  0,  0, 0; 
%            0,  0,  0, 0; 
%            0,  0,  0, 0
%          ];
     
     
     
% Noise... 
Mprev = [0;0;0;0]; %[10;10;10;10];
sigmaM = [1;1;1;1]; %[10; 10; 10; 10];
sigmaT = [1;1;1;1]; %[25;25;25;25];

for i = 1:16
    t(1) = t(1); % simulate a tempo change from the input tapper 
    [tNext(:, i), Mn] = LinearPhaseCorrection(t, TkInterval, Mprev, alphas, sigmaM, sigmaT);
    Mprev = Mn;

    %... and repeat...
    t = tNext(:, i);
    x(:, i) = t;
end



c = {'k', 'r', 'b', 'g'};
figure()
for i = 1:size(x, 1)
    plot(diff(x(i, :)), ['-x',c{i}],'LineWidth', 3);     
    hold on; grid on;
end
title('IOI Correction');
legend('p1', 'p2', 'p3', 'p4');
ylabel('mean IOI (s)');
xlabel('Event Num');