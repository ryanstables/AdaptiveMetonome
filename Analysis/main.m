clear;clc

%%
savedData;

%%
% choose trial 
x= x_0;
% x= x_1;
% x= x_2;

%% Plot the output of the 
fs = 44100;
bpm = 60;

subplot(2, 2, 1);
clrz = {'k', 'r', 'g', 'b'};
for i = 1:size(x, 2)
    plot(x(:, i)/fs, zeros(length(x))+(i*0.2), ['d',clrz{i}], 'MarkerSize', 25); 
    hold on; grid on;
end
xlabel('Event Time (S)')
ylabel('Tappers')

subplot(2, 2, 2);
for i = 1:size(x, 2)
    plot(x(:, i)/fs, ['-x',clrz{i}]);     
    hold on; grid on;
end
ylabel('Event Time (S)')
xlabel('Event Num')


subplot(2, 2, 3);
for i = 1:size(x, 2)
    plot(diff(x(:, i))/fs, ['-x',clrz{i}]);     
    hold on; grid on;
end
ylabel('mean IOI (s)');
xlabel('Event Num');

subplot(2, 2, 4);
plot((mean(x(:, 2:4), 2)-x(:, 1))/fs, 'x-k');
ylabel('input Async (s)');
xlabel('Event Num');
 
    