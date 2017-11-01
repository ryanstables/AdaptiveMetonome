%example script

% t = [0;0;0;0];
TkPeriod = 500;
Mprev = [10;10;10;10];
alphas = [0,.3,.3,.3; 
          .2, 0, .2, .2; 
          .1, .1, 0, .1; 
          .5, .5, .5, 0];
      
sigmaM = [10; 10; 10; 10];
sigmaT = [25;25;25;25];
[tNext, Mn] = nextTapsLPC(t, TkPeriod, Mprev, alphas, sigmaM, sigmaT);

Mprev = Mn;
t = tNext

%... and repeat...



