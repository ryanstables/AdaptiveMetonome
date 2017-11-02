function [tNext, Mn] = LinearPhaseCorrection(t, TkInterval, Mprev, alphas, sigmaM, sigmaT)

N = length(t); 

Tn= sigmaT.*randn(length(sigmaT),1); % ...Timekeeper Noise
Mn= sigmaM.*randn(length(sigmaM),1);  % ...Motor Noise
Hn= Tn+Mn - Mprev; % ...noise at tap(n)

% allocate arrays...
async = zeros(N); AsyncAlpha = zeros(N);
tNext = zeros(N, 1);  A = zeros(N, 1); 

% player i tap relative to player j...
for i=1:N
    for j=1:N
        async(i, j) = t(i) - t(j);
        AsyncAlpha(i, j) = alphas(i, j) * async(i, j);        
    end    
    A(i) = sum(AsyncAlpha(i, :));
    tNext(i) = t(i) + TkInterval - A(i) + Hn(i);       
end
