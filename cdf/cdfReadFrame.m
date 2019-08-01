function frame = cdfReadFrame(fileID)
    index = fread(fileID,1,'uint');
    headerSize = fread(fileID,1,'uint');
    xlen = fread(fileID,1,'uint');
    ylen = fread(fileID,1,'uint');
    header = fread(fileID,headerSize,'char')';
    data = fread(fileID,[xlen,ylen],'float')';    
    frame = struct('index',index,'headerSize',headerSize,'xlen',xlen,'ylen',ylen,'header', header,'data',data);
end