function frames = cdfGetFrames(header)
    frames = cell(header.numFrames,1);
    %pos = 24 + header.variableHeaderSize;
    %24 is size of fixed header, 512 is size of ascii header
    pos = 512+24+header.variableHeaderSize;
    fseek(header.fileID,pos,'bof');
    for a = 1:header.numFrames
        frames{a} = cdfReadFrame(header.fileID);    
    end
    
end
        