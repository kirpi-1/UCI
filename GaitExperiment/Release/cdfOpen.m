function header = cdfOpen(filename)
    
    %opens a file and reads the header
    fin = fopen(filename);
    asciiHeader = fread(fin,512,'char');
    asciiHeader=char(asciiHeader.*(asciiHeader<=122 & asciiHeader>=32));
    numFrames = fread(fin,1,'uint');
    variableHeaderSize = fread(fin,1,'uint');
    subNum = fread(fin,1,'ushort');
    min = fread(fin,1,'uint8');
    hour = fread(fin,1,'uint8');
    day = fread(fin,1,'uint8');
    month = fread(fin,1,'uint8');
    year = fread(fin,1,'ushort');
    type = fread(fin,1,'ushort');
    numElec = fread(fin,1,'ushort');
    numFreq = fread(fin, 1,'ushort');
    numCoi = fread(fin,1,'ushort');
    numActions = fread(fin,1,'ushort');
    trialType=fread(fin,1,'ushort');
    freqs = fread(fin,numFreq,'float');
    coiFilename = fread(fin,64,'char')';
    coiFilename = char(coiFilename.*(coiFilename<=122 & coiFilename>=32));
    actionFilename = fread(fin,64,'char')';
    actionFilename = char(actionFilename.*(actionFilename<=122 & actionFilename>=32));
    coiNames = fread(fin,[16,numCoi],'char')';
    coiNames = char(coiNames.*(coiNames<=122 & coiNames>=32));
    actionNames = fread(fin,[16,numActions],'char')';
    actionNames = char(actionNames.*(actionNames<=122 & actionNames>=32));
    cois = fread(fin,[numElec,numCoi],'float');
    actions = zeros(numFreq,numCoi,numActions);
    for i=1:numActions
        actions(:,:,i) = fread(fin,[numFreq,numCoi],'float');
    end
    header = struct('fileID',fin,'asciiHeader',asciiHeader','numFrames',numFrames,...
        'variableHeaderSize',variableHeaderSize,'subNum',subNum,...
        'min', min,'hour',hour,'day',day,'month',month,'year',year,'type',type,...
        'numElec',numElec,'numFreq',numFreq,'numCoi',numCoi,...
        'numActions',numActions,'trialType',trialType,'freqs',freqs,'coiFilename',coiFilename,...
        'actionFilename',actionFilename, 'coiNames',coiNames,...
        'actionNames',actionNames,'cois',cois,'actions',actions);
end