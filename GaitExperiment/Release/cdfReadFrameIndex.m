function frame = cdfReadFrameIndex(header,index,varargin)
    %28 is the size of the fixed header
    %bof stands for beginning of file
    fseek(header.fileID,header.variableHeaderSize+28,'bof');
    %fseek(header.fileID,header.variableHeaderSize+28+512,'bof');
    frame = cdfReadFrame(header.fileID);
    while(frame.index~=index)
        frame = cdfReadFrame(header.fileID);
    end
    
end