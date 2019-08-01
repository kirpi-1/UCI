function frame = cdfReadFrame(fileID)
    index = fread(fileID,1,'uint');
    footerSize = fread(fileID,1,'uint');
    xlen = fread(fileID,1,'uint');
    ylen = fread(fileID,1,'uint');
    timestamp = fread(fileID,1,'uint');    
    data = fread(fileID,[xlen,ylen],'float')';    
    footer= fread(fileID,footerSize,'uint8')';
    frame = struct('index',index,'footerSize',footerSize,'xlen',xlen,'ylen',ylen,'timestamp',timestamp,'footer',footer,'data',data);
end