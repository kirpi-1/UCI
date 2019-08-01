function block = getBlock()
n = 9;
conditions = [1:n];
global trials blockStr
trials = zeros(n,n);
blockStr = int2str(block);

for i = 1:n
    thisCond = ceil(rand*n)   
    s = findPos(thisCond,1,n);    
end

end

function success = findPos(num,curRow,maxSize)
    global trials blockStr    
    exit = false;
    col = 1;
    success = false;
    while(~exit)
        %if this square is empty, go to next check
        if trials(curRow,col)== 0            
            %if this column doesn't have this number in it, go to next
            %check
            if sum(trials(:,col)==num)==0
                %finally, check that if I put this number here, I wouldn't
                %have the same sequence twice in the whole block
                %first, create sequence you are looking for
                seq = [];
                if col == 1
                    seq = int2str([num,trials(curRow,col+1)]);
                else
                    seq = int2str([trials(curRow,col-1),num]);
                end
                %next, try to find it
                found = [];
                for i=1:maxRows
                    found = strfind(blockStr(i,:),seq);
                    if ~isempty(found)
                        break;
                    end
                end
                %if I didn't find it, then place the number in the col and
                %go to the next row
                if isempty(found)
                    
                
            end
        end
        
        
        if col>maxSize
            exit = true
        end
    end



end