% Problem 
% - if an input happens after the others, it's counted as the next beat...
% - isn't this what the window is for? 

%% Load data...
clear;clc
cd '../Data';
savedData();
cd '../Analysis/';


%% choose trial 
% x = tNext'
x= x_0;
% x= x_1;
% x= x_2;

fs = 44100;
bpm = 60;

subplot(2, 2, 1);
clrz = {'k', 'r', 'g', 'b'};
for i = 1:size(x, 2)
    plot(x(:, i)/fs, zeros(length(x))+(i*0.2), ['d',clrz{i}], 'MarkerSize', 25);     
    hold on;
end
for i = 1:size(x, 1)
    m = mean(x(i,:))/fs;
    plot([m m], [0 1], '--k'); hold on;
end
title('Onset Times');
xlabel('Event Time (S)')
ylabel('Tappers')

subplot(2, 2, 2);
for i = 1:size(x, 2)
    plot(x(:, i)/fs, ['-x',clrz{i}]);     
    hold on; grid on;
end
title('Temporal Variance');
ylabel('Event Time (S)')
xlabel('Event Num')


subplot(2, 2, 3);
for i = 1:size(x, 2)
    plot(diff(x(:, i))/fs, ['-x',clrz{i}]);     
    hold on; grid on;
end
title('IOI Correction');
legend('p1', 'p2', 'p3', 'p4');
ylabel('mean IOI (s)');
xlabel('Event Num');

subplot(2, 2, 4);
plot((x(:, 1) - mean(x(:, 2:4), 2))/fs, 'x-k'); grid on;
title('P1 vs Rest Asynchrony')
ylabel('input Async (s)');
xlabel('Event Num');
 
    