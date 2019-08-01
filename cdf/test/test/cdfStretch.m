% splits the data from the frame into spectral data (eegData), rawEEG, and
% cube data (if it exists)
function [eegData cubeData rawEEGData]= cdfStretch(header, frames, cubes)
    eegData = zeros(header.numCoi,header.numFrames,header.numFreq);
    %rawEEGData = zeros(header.numElec,1);
    rawEEGData = cell(header.numFrames,1);
    cubeData = zeros(6,header.numFrames);
    for i=1:header.numFrames
        for j=1:header.numFreq
            eegData(:,i,j) = frames{i}.data(:,j);
        end
        if frames{i}.headerSize>0;            
            rs = [];
            if isempty(cubes)                
                rs = reshape(frames{i}.header,frames{i}.headerSize/header.numElec,header.numElec)';
            else
                cubeData(:,i) = frames{i}.header(1:6);
                if frames{i}.headerSize>6
                    rs = reshape(frames{i}.header(7:end),(frames{i}.headerSize-6)/header.numElec,header.numElec)';
                end
            end
            %rawEEGData = horzcat(rawEEGData,rs);            
            rawEEGData{i} = rs;            
        else
            %cubeData(:,i) = zeros(6,1);
        end
       
    end
end
