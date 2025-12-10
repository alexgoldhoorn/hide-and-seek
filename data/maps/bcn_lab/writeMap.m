function writeMap(map,outfile)
%writeMap
% map: matrix int with 0=free, 1=obstacle, 2=base
% outfile: output file

fid=fopen(outfile,"w");


if fid<0
	error(['failed to open file ' outfile ]);
	exit(-1)
end

s=size(map);
fprintf(fid,'%d,%d,\n',s(2),s(1));

    for r=1:s(1)
        for c=1:s(2)
		fprintf(fid,"%d",map(r,c));
	end
	fprintf(fid,"\n");
    end

fclose(fid);

disp(['Map correctly saved to ' outfile]);


end

