function frame = cdfReadFrame(fileID)
    index = fread(fileID,1,'uint');
    headerSize = fread(fileID,1,'uint');
    xlen = fread(fileID,1,'uint');
    ylen = fread(fileID,1,'uint');
    timestamp = fread(fileID,1,'uint32');
    data = fread(fileID,[xlen,ylen],'float')';
    header = fread(fileID,headerSize,'uint8')';
    %can use typecast(uint8(header),'int32');
    frame = struct('index',index,'headerSize',headerSize,'xlen',xlen,'ylen',ylen,'timestamp',timestamp,'header', header,'data',data);
end