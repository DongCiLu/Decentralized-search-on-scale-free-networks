#include <iostream>

#include "Snap.h"

using namespace std;

int main(int argc, char** argv){
    // loading graph from edgelist
    cout << "1. Loading graph" << endl;
    PUNGraph net = TSnap::LoadEdgeList<PUNGraph>(argv[1], 0, 1);
    TSnap::PrintInfo(net);
    cout << "2. Searching for wcc and delete self edge" << endl;
    net = TSnap::GetMxWcc<PUNGraph>(net);
    TSnap::DelSelfEdges(net);
    cout << "3. Gather info" << endl;
    TSnap::PrintInfo(net);
    cout << "4. Save wcc" << endl;
    TSnap::SaveEdgeList<PUNGraph>(net, argv[2]);

    return 0;
}

