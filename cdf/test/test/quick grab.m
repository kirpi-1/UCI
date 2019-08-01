coi1 = [];
coi2 = [];
coi3 = [];
coi4 = [];
for i=1:500
    coi1 = horzcat(coi1,frame{i}.data(1,:));
    coi2 = horzcat(coi1,frame{i}.data(2,:));
    coi3 = horzcat(coi1,frame{i}.data(3,:));
    coi4 = horzcat(coi1,frame{i}.data(4,:));
end