fclose('all');
h= cdfOpen('sub26_trial00_2014_11_07_18_59.cdf')
frames = cdfGetFrames(h);

hmd = zeros(h.numFrames,9);
timestamps = zeros(h.numFrames,1);
data = [];
for a=1:h.numFrames
hmd(a,1) = typecast(uint8(frames{a}.header(1:4)),'uint32');
hmd(a,2) = typecast(uint8(frames{a}.header(5:8)),'uint32');
hmd(a,3:end) = typecast(uint8(frames{a}.header(9:end)),'single');
timestamps(a) = frames{a}.timestamp;
data = vertcat(data,frames{a}.data);
end

%interpolation
X = 1:length(hmd);
XX = linspace(1,length(X),length(data));
hmdinterpol = zeros(length(data),size(hmd,2));
for a=1:size(hmd,2)
hmdinterpol(:,a) = spline(X,hmd(:,a),XX);
end