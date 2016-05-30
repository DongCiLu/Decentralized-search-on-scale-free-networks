#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main(int argc, char** argv) {
    ifstream ifile(argv[1]);

    vector<string> lines;
    string line;
    while(getline(ifile, line)) {
        lines.push_back(line);
    }
    ifile.close();

    random_shuffle(lines.begin(), lines.end());

    ofstream ofile(argv[1]);
    for (size_t i = 0; i < lines.size(); i++) {
        ofile << lines[i] << endl;
    }
    ofile.close();

    return 0;
}
