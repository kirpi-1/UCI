function data = cdfGetCOIData(header,varargin)
    if nargin==1
       frames = cdfGetFrames(header);
       for i=1:header.numCoi           
           data.(header.coiNames(i,:)) = zeros(header.numFreq,header.numFrames);
           for j=1:header.numFrames
              data.(header.coiNames(i,:))(1:header.numFreq,j)=frames{j}.data(:,i);
           end
       end
        
        
    else 
        
        
    end
end