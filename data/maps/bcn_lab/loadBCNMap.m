function map=loadBCNMap(mapfile,reqStepSize,selXStart,selXEnd,selYStart,selYEnd)
%load map file 
%according to def of bcn map

%reqStepSize: required step size

    

obst = -2;
mapvec = load(mapfile);
% min/max x,y of the map in meters
minX = mapvec(1);
maxX = mapvec(2);
minY = mapvec(3);
maxY = mapvec(4);
%step size in  meters
step = mapvec(5);

if nargin==1 || reqStepSize==0
    reqStepSize = step;
end
reqStepSize
    
    
%# rows, cols
rows = (maxY-minY)/step
cols = (maxX-minX)/step

data = mapvec(6:length(mapvec));
%reshape vector -> map
mapZ = reshape(data,cols,rows)';




%generate map
map = zeros(size(mapZ));
%set obst
map(mapZ==-2)=1;

%flips map
map = flipud(map);

if (nargin>2) 
   %select certain part based on index!!!
   ixs = selXStart;  %floor(selXStart/step)
   ixe = selXEnd;  %ceil(selXEnd/step)
   iys = selYStart;  %floor(selYStart/step)
   iye = selYEnd;  %ceil(selYEnd/step)
   
   map = map([iys:iye],[ixs:ixe]); 
   
   [rows cols] = size(map);
end

%now convert step to reqStepSize
stepFact = reqStepSize/step

if reqStepSize~=step

    newRows = ceil(rows/stepFact)
    newCols = ceil(cols/stepFact)

    sf = round(stepFact/2)
    [orows ocols] = size(map);

    % generate new map 
    % for each cell average the old cells with a square of stepFact^2 
    mapN = zeros(newRows,newCols);
    for r=1:newRows
        for c=1:newCols
            v=0;
            n=0;
            %now sum values in neighbourhood
            for or=(round(stepFact*r)-sf:round(stepFact*r)+sf)
                if (or<=0 || or>orows)  % check coord ok
                    continue;
                end
                for oc=(round(stepFact*c)-sf:round(stepFact*c)+sf)
                    if (oc<=0 || oc>ocols) % check coord
                        continue;
                    end
                    % add value and count
                    v = v + map(or,oc);
                    n = n + 1;                    
                end
            end

            %set avg        
            mapN(r,c)= round(v/n);
        end
    end


    map =mapN;

end



imagesc(map);

