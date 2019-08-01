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
    trialType=fread(fin,1,'ushort');
    junk = fread(fin,2,'char'); %buffer so that the structure is 24bytes
    if(variableHeaderSize>0)
        variableHeader = fread(fin,variableHeaderSize,'char');
    else
        variableHeader = [];
    end
    header = struct('fileID',fin,'asciiHeader',asciiHeader','numFrames',numFrames,...
        'variableHeaderSize',variableHeaderSize,'subNum',subNum,...
        'min', min,'hour',hour,'day',day,'month',month,'year',year,'type',type,...
        'numElec',numElec,'trialType',trialType,'variableHeader',variableHeader);
end