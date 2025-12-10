

#include "HSGame/gmap.h" 


#include <iostream>
#include <cstdlib>

using namespace std;




int main(int argc, char *argv[])
{

	if (argc<6) {
		cout << "Required parameters:"<<endl<<" image.pgm map_out.txt zoom-out base_row base_col"<<endl;
		return 0;
	}

	const char* pgmFile = argv[1];
	const char* outFile = argv[2];

	cout 	<< "PGM file: " << pgmFile<<endl
		<< "Out file: " << outFile<<endl;

	int zoomOutF = atoi(argv[3]);
	int baseRow = atoi(argv[4]);
	int baseCol = atoi(argv[5]);


	cout 	<< "Zoom out: " << zoomOutF<<endl
		<< "Base:     r" << baseRow<<"c"<<baseCol <<endl;

	cout << "Loading PGM map: "<<flush;
	Pos p(baseRow,baseCol);
	GMap gmap(pgmFile, p, zoomOutF);
	cout <<"done"<<endl<<"Writing "<<outFile<< " ..."<<flush;
	gmap.writeMapFile(outFile);

	cout << "Done"<<endl;

	
}
