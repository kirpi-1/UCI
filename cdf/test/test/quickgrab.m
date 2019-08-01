coi1 = [];
coi2 = [];
coi3 = [];
coi4 = [];
for i=1:h.numFrames
    coi1 = horzcat(coi1,frame{i}.data(:,1));
    coi2 = horzcat(coi2,frame{i}.data(:,2));
    coi3 = horzcat(coi3,frame{i}.data(:,3));
    coi4 = horzcat(coi4,frame{i}.data(:,4));
    i
end