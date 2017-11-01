function [tNext, Mn] = nextTapsLPC(t, TkPeriod, Mprev, alphas, sigmaM, sigmaT)
% Assume N tappers
% t is the previous tap times (Nx1) vector
% TkPeriod is the underlying tempo, scalar value
% Mprev is the previous motor delay, (Nx1) vector
% alphas are the correction gains between players (NxN) matrix
% sigmaM, sigmaT motor, Timekeeper standard deviations for each player (Nx1) vectors

% Returns:
% tNext the next tap times for each player (Nx1)
% Mn the current motor delay


Tn= TkPeriod + sigmaT.*randn(length(sigmaT),1); %Next timekeeper interval 
Mn=sigmaM.*randn(length(sigmaM),1); %Next motor delay

Hn=Tn+Mn - Mprev; % Calculate tap onset delay based on timekeeper and motor delays

% player ii tap relative to player jj cue
for ii=1:length(t)
    for jj=1:length(t)
        A(ii,jj) = t(ii) - t(jj); %calc Async between each player
        AsyAlpha2(ii,jj) = A(ii,jj).*alphas(ii,jj);
    end 
end

AsyAlpha = alphas .* A; %calc each alpha by associated Asynchrony

% tNext = t + AsyAlpha(ii, :)' + Hn; %calculate next tap time
tNext = t + sum(AsyAlpha)' + Hn;

for tapper = 1:length(t)
    tNext2(tapper) = t(tapper) + sum(AsyAlpha(:, tapper)) + Hn(tapper);
end

