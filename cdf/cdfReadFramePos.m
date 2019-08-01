function [frame pos] = cdfReadFramePos(header, pos)
    fseek(header.fileID,pos,'bof');
    frame = cdfReadFrame(header.fileID);    
    pos = ftell(header.fileID);
end
    