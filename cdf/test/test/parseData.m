function [trialsEEG,trialsCube] = parseData(session)
close 'all'
fig = figure('Position',[100 100 1480 740]);
hold on;
plot(session.cubeData(2,:),'r');
plot(session.cubeData(5,:),'b');
[start y] = ginput(1);
start = ceil(start);
clippedCubeData = session.cubeData(:,start:end);
m = mean(clippedCubeData(:,:),2);
for i=1:6
    clippedCubeData(i,:) = clippedCubeData(i,:) - m(i);
end
clippedEEGData = session.eegData(:,start:end,:);

close(fig);
fig = figure;
hold on;
%leg going sideways
plot(clippedCubeData(2,:),'r');
plot(clippedCubeData(5,:),'b');

fig = figure('Position',[100 100 1480 740]);
hold on;
%leg swinging
plot(clippedCubeData(3,:),'r');
plot(clippedCubeData(6,:),'b');


trialsCube = cell(session.totalNumTrials,1);
trialsEEG = cell(session.totalNumTrials,1);
trialLength = 250;
for i = 1:session.totalNumTrials;
    %for each trial
    absLeftCube = abs(clippedCubeData(3,:));
    absRightCube = abs(clippedCubeData(6,:));
    searchData = [];
    numPeaks = 0;
    %get which type it was, mark how many peaks are needed and which cube
    %the mark is coming from
    if session.block(i) <4; % slow w/ click, right 1
        searchData = absRightCube; 
    else
        searchData = absLeftCube;
    end
    if session.block(i) == 1 || session.block(i) == 4
        numPeaks = 1;
    elseif session.block(i) == 2 || session.block(i)==5
        numPeaks = 2;
    else
        numPeaks = 3;
    end
    %find all the peaks and their locations from the appropriate cube
    [peaks locs] = findpeaks(searchData);
    possibleLocs = ceil(locs(peaks>20));
    
    %find the start of the trial markings
    j=0;
    response = 'No';
    while strcmp(response,'No')
        j = j+1;
        hold off
        plot(clippedCubeData(3,1:500),'r');
        hold on
        plot(clippedCubeData(6,1:500),'b');
        line([possibleLocs(j) possibleLocs(j)],[-80 80],'color','k');
        response = questdlg('Is the the start of the trial markings?');
        if strcmp(response,'Cancel')
            return
        end
    end    
    %go to the first peak after the marking peaks, this is the start of the
    %trial data
    trialStart = possibleLocs(j + numPeaks);
    trialEnd = trialStart+trialLength;
    l = length(clippedCubeData(3,trialStart:end));
    plotStart = trialStart;
    plotEnd = plotStart+750;
    if l<750; plotEnd = plotStart+l-1; end
    
    %plot from the trial start to about 750 units afterwards
    hold off
    plot(clippedCubeData(3,plotStart:plotEnd),'r');
    hold on
    plot(clippedCubeData(6,plotStart:plotEnd),'b');
    
    %plot a line at the proposed trial ending
    line([trialLength trialLength],[-80 80],'color','k');    
    response = questdlg('Is this a good ending spot?');
    if strcmp(response,'Cancel'); return;
    elseif strcmp(response,'No')
        [trialLength, endy] = ginput(1);
        trialLength = ceil(trialLength);
    end
    
    trialEnd = trialLength + trialStart;
    %put that trial in its own cell
    trialsCube{i} = clippedCubeData(:,trialStart:trialEnd);
    trialsEEG{i} = clippedEEGData(:,trialStart:trialEnd,:);
    %clip out the trial you just pulled from the cubeData
    clippedCubeData = clippedCubeData(:,trialEnd+1:end);
    clippedEEGData = clippedEEGData(:,trialEnd+1:end,:);
    
    if(i<session.totalNumTrials)        
        %find the waiting period for the next trial
        start = 1;
        found = false;
        while ~found        
            l = length(clippedCubeData(3,start:end));
            if l>50
                l=50;
            end
            cLeft = mean(abs(clippedCubeData(3,start:start+l-1)));
            cRight = mean(abs(clippedCubeData(6,start:start+l-1)));
            if (cRight+cLeft)/2 < 10
                plotStart = start-400;
                if plotStart<1; plotStart=1; end
                plotEnd = start+400;
                if plotEnd - plotStart > length(clippedCubeData(3,plotStart:end))
                    plotEnd = plotStart + length(clippedCubeData(3,plotStart:end))-1;
                end
                hold off
                plot(clippedCubeData(3,plotStart:plotEnd),'r');
                hold on
                plot(clippedCubeData(6,plotStart:plotEnd),'b');

                %plot a line at the proposed trial ending
                line([start-plotStart+25 start-plotStart+25],[-80 80],'color','k');
                pause(0.001);
                response = questdlg('Is this the waiting period?','','Yes','No','Go Back','Yes');
                if strcmp(response,'Cancel')
                    return
                elseif strcmp(response,'Yes')
                    found = true;                
                elseif strcmp(response,'Go Back')
                    start = start-50;
                end
                
            end
            start=start+25;
        end
        clippedCubeData = clippedCubeData(:,start:end);
        clippedEEGData = clippedEEGData(:,start:end,:);
    end   
    
end


