n = 9;
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
conditions = cell(9,1);
conditions{1} = 'slow w/ tempo click';
conditions{2} = 'med w/ tempo click';
conditions{3} = 'fast w/ tempo click';
conditions{4} = 'slow w/o tempo click';
conditions{5} = 'med w/o tempo click';
conditions{6} = 'fast w/o tempo click';
conditions{7} = 'slow no attention';
conditions{8} = 'med no attention';
conditions{9} = 'fast no attention';

for i=1:9
    [textBlock{rblock==i}] = deal(conditions{i});
end


