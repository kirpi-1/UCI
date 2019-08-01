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
block

%1 - slow w/ tempo click
%2 - med w/ tempo click
%3 - fast w/ tempo click
%4 - slow w/o tempo click
%5 - med w/o tempo click
%6 - fast w/o tempo click
%7 - slow no attention
%8 - med no attention
%9 - fast no attention
