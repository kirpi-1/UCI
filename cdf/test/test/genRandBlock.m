n = 6;
pool = [1:n];
block = zeros(n,n);

start = ceil(rand*n);
for i=1:n
    block(i,1) = start;
    for j=2:n
        remainingPool = setdiff(pool,block(i,:));
        block(i,j) = remainingPool(ceil(rand*(n-j+1)));
    end
    if i<n
        nextStartPool = setdiff(pool,[block(i,n);block(:,1)]);
        start = nextStartPool(ceil(rand*length(nextStartPool)));
    end
end
rblock = reshape(block',n*n,1);
textBlock = cell(n*n,1);
conditions = cell(n,1);
conditions{1} = 'slow w/ tempo click 60bpm';
conditions{2} = 'med w/ tempo click 90bpm';
conditions{3} = 'fast w/ tempo click 120bpm';
conditions{4} = 'slow w/o tempo click 60bpm';
conditions{5} = 'med w/o tempo click 90bpm';
conditions{6} = 'fast w/o tempo click 120bpm';
%conditions{7} = 'slow no attention';
%conditions{8} = 'med no attention';
%conditions{9} = 'fast no attention';

for i=1:n
    [textBlock{rblock==i}] = deal(conditions{i});
end
    

