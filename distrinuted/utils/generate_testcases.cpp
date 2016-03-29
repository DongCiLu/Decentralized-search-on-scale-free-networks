#include <fstream>
#include <set>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>


using namespace std;

int main(int argc, char **argv) {
    ifstream ifile(argv[1]);

    set<unsigned long> vset;
    unsigned long vid = 0;
    string line;
    getline(ifile, line);
    cout << line << endl;
    getline(ifile, line);
    cout << line << endl;
    getline(ifile, line);
    cout << line << endl;
    while(getline(ifile, line)) {
        stringstream ss(line);
        while(ss >> vid) 
            vset.insert(vid);
    }
    ifile.close();

    ofstream ofile(argv[2]);
    srand(time(NULL));
    size_t total = vset.size() * 2;
    size_t highest = *(vset.rbegin());
    cout << vset.size() << " " << highest << endl;
    unsigned long src_id, dst_id;
    for(size_t i = 0; i < 1000; i++) {
        do {
            src_id = rand() % total;
        } while (vset.find(src_id) == vset.end());

        for (size_t j = 0; j < 1000; j++) {
            do {
                dst_id = rand() % total;
            } while (vset.find(dst_id) == vset.end());
            ofile << src_id << " " << dst_id << endl;
        }
    }
    ofile.close();
            

    return 0;
}
