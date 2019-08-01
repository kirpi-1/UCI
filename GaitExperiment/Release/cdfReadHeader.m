function header = cdfReadHeader(filename)
    
    %opens a file and reads the header
    fin = fopen(filename);
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
    numcoi = fread(fin,1,'ushort');
    numactions = fread(fin,1,'ushort');
    padding=fread(fin,2,'char');
    coinames = fread(fin,[16,numcoi],'char')';
    coinames=char(coinames.*(coinames<=122 & coinames>=32));
    actionnames = fread(fin,[16,numcoi],'char')';
    actionnames = char(actionnames.*(actionnames<=122 & actionnames>=32));
    cois = fread(fin,[numElec,numcoi],'float');
    actions=zeros(numFreq,numcoi,numactions);
    for i=1:numactions
        actions(:,:,i) = fread(fin,[numFreq,numcoi],'float');
    end
    header = struct('fileHandle',fin,'numFrames',numFrames,...
        'variableHeaderSize',variableHeaderSize,'subNum',subNum,...
        'min', min,'hour',hour,'day',day,'month',month,'year',year,'type',type,...
        'numElec',numElec,'numFreq',numFreq,'numCoi',numcoi,...
        'numActions',numactions,'coiNames',coinames,...
        'actionNames',actionnames,'cois',cois,'actions',actions);
end