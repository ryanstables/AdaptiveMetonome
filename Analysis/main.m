%% Plot the output of the 
fs = 44100;
bpm = 70;
beatInterval = (1/(bpm/60))*fs;

onsts = Onsets/fs; 
intrs = Intervals/fs;
means = Means/fs;
metro = (0:beatInterval:(beatInterval*length(Means))-1)/fs;
metro = metro + min(means); %offset the metronome to compare against the means

% plot the onset locations...
clrz = {'r','g','b'};
for i=1:3;
    x = onsts(:, i);
    plot(x, zeros(length(x))+(i*0.2), ['d',clrz{i}], 'MarkerSize', 25); hold on;grid on;
end

% plot the means and the metronome...
for i =1:length(Means) 
    plot([means(i), means(i)], [0, 4], 'k'), hold on;
%     plot([metro(i), metro(i)], [0, 4], 'r--'), hold on; % plot to see how much tempo deviation there is! 
end

xlabel('Time (Sec)');
ylabel('Tappers');
axis([min(min(onsts)), max(max(onsts)), 0, 0.2*4])

%% Debugging: Check some details to ensure the code is working...
if ~sum(sum(Intervals(1:end-1, :) - diff(Onsets)))
   fprintf('\nIntervals = beat differentials\n'); 
end
fprintf('random values: ');
for i = 1:3
    fprintf('%i ', Intervals(1, i)-beatInterval);
end
    fprintf('\n');
    
