while true
    f = fopen('C:\Users\CNS Lab\Documents\Visual Studio 2010\Projects\GaitExperiment\GaitExperiment\bufferOutput.txt')
    a = fscanf(f,'%f');
    fclose(f);
    plot(a);
    ylim([min(a)-.1*abs(min(a)),max(a)+.1*(abs(max(a)))]);
    pause(5);
end