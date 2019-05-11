#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <cstdlib>

#include "Snap.h"

using namespace std;

const int NUM_SRC = 1000;
const int NUM_DST = 1000;

struct Testcase {
    int src;
    int dst;
    int dist;

    Testcase(int src, int dst, int dist) : 
        src(src), dst(dst), dist(dist) {}
};

int main(int argc, char **argv) {
    // read edge list
    cout << "1. Loading graph" << endl;
    PUNGraph net = TSnap::LoadEdgeList<PUNGraph>(argv[1], 0, 1);
    TSnap::PrintInfo(net);

    cout << "2. Select random source vertex and perform BFS" << endl;
    vector<Testcase> testcases;
    for (int i = 0; i < NUM_SRC; i ++) {
        int src = net->GetRndNId();
        unordered_map<int, int> dset;
        // generate random dst
        for (int j = 0; j < NUM_DST; j ++) {
            int dst;
            do {
                dst = net->GetRndNId();
            } while (dst == src || dset.find(dst) != dset.end());
            dset.insert(make_pair(dst, -1));
        }
        // perform BFS to find dist for each dst
        unordered_set<int> visited;
        queue<pair<int, int>> fifo;
        fifo.push(make_pair(src, 0));
        visited.insert(src);
        while(!fifo.empty()) {
            int cur =  fifo.front().first;
            int dist =  fifo.front().second;
            fifo.pop();
            if (dset.find(cur) != dset.end())
                dset[cur] = dist;
            for (int i = 0; i < net->GetNI(cur).GetDeg(); i ++) {
                int nid = net->GetNI(cur).GetNbrNId(i);
                if (visited.find(nid) == visited.end()) {
                    fifo.push(make_pair(nid, dist + 1));
                    visited.insert(nid);
                }
            }
        }
        // create testcases;
        for (auto entry: dset) {
            testcases.push_back(Testcase(
                        src, entry.first, entry.second));
        }
        cout << "." << flush;
    }
    cout << endl;
            
    cout << "3. Output to file" << endl;
    ofstream ofile(argv[2]);
    srand(time(NULL));
    while(!testcases.empty()) {
        int r = rand() % testcases.size();
        swap(testcases[r], testcases.back());
        ofile << testcases.back().src << "  " 
              << testcases.back().dst << "  "
              << testcases.back().dist << endl;
        testcases.pop_back();
    }
    ofile.close();

    return 0;
}
