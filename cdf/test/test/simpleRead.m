h = cdfOpen('test1.cdf');
f = cdfGetFrames(h);

session.block = [1];
[session.eegData,session.cubeData] = cdfStretch(h,f);
session.numConditions = 6;
session.totalNumTrials = 12;
%eeg = session.eegData(:,3200:end,:);
%cube = session.cubeData(:,3200:end,:);
%[eegTrials, cubeTrials] = parseData(session)

