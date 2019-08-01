function frames = cdfGetFrames(header,varargin)
    start = [];
    stop = [];
    if nargin == 1
        start = 1;
        stop = header.numFrames;
    elseif nargin == 2
        start = varargin{1};
        stop = header.numFrames;
    elseif nargin == 3
        start = varargin{1};
        stop = varargin{2};
    end
    frames = cell(stop-start,1);
    %pos = 28 + header.variableHeaderSize;
    %28 is size of fixed header, 512 is size of ascii header
    pos = 512+28+header.variableHeaderSize;
    for i = start:stop
        [frames{i-start+1} pos] = cdfReadFramePos(header,pos);
    end
    
end
        