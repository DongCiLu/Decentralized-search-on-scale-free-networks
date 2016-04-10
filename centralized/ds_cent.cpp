/* 
 * Author: Zheng Lu
 * Date: Jul. 15, 2014
 * Description: Maximum information multitree for code system.
 */

#include <iostream>
#include <limits>
#include <time.h>
#include "ds_cent.h"

using namespace std;

template <typename id_type, typename dist_type>
string ds_cent<id_type, dist_type>::get_time() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%d-%X", &tstruct);
    return buf;
}

template <typename id_type, typename dist_type>
ds_cent<id_type, dist_type>::ds_cent(string graphfile) : 
    total_tick(0), total_comp_tick(0), total_path_cnt(0),
    total_real(0), total_est_all(0), total_est_multi(0), 
    total_est(0), total_comp(0), total_obv(0),
    num_tree(2), num_exp(100000) {
    // loading graph from edgelist
    net = TSnap::LoadEdgeList<PGRAPH_TYPE>(graphfile.c_str(), 0, 1);
    max_dist = net->GetNodes();
    size_t max_exp = max_dist * max_dist;
    num_exp = min(num_exp, max_exp);

    root_id.resize(num_tree, 0);
    vector< vector<id_type> > init_code(num_tree); 
    for (TGRAPH_TYPE::TNodeI NI = net->BegNI(); NI < net->EndNI(); NI++) {
        codes.insert(make_pair(NI.GetId(), init_code));
        unlabeled_degree[NI.GetId()] = NI.GetDeg();
    }

    size_t pos = graphfile.find_last_of("/") + 1;
    graphname = graphfile.substr(pos, graphfile.size()-pos-8);
    string ofilename = graphname + "-" + get_time() + ".txt";
    out.open(ofilename.c_str());
}

template <typename id_type, typename dist_type>
ds_cent<id_type, dist_type>::~ds_cent() {
    out.close();
}

template <typename id_type, typename dist_type>
dist_type ds_cent<id_type, dist_type> ::
get_dist(id_type src_id, id_type dst_id, id_type& lca) {
    dist_type dist = max_dist; 
    for (size_t t = 0; t < num_tree; t++) {
        size_t range = min(codes[src_id][t].size(), 
                codes[dst_id][t].size());
        size_t i = 0;
        id_type lca_t = codes[src_id][t][0];
        while (i < range) {
            if (codes[src_id][t][i] != codes[dst_id][t][i])
                break;
            lca_t = codes[src_id][t][i];
            i ++;
        }
        dist_type code_dist = codes[src_id][t].size() 
            + codes[dst_id][t].size() - 2*i;
        if (i != 0 && code_dist < dist) {
            dist = code_dist;
            lca = lca_t;
        }
    }

    return dist;
}

template <typename id_type, typename dist_type>
id_type ds_cent<id_type, dist_type>::select_root(size_t t) {
    size_t max_deg = 0;
    id_type next_root_id = -1;
    for (TGRAPH_TYPE::TNodeI NI = net->BegNI(); NI < net->EndNI(); NI ++){
        id_type id = NI.GetId();
        size_t root_deg = 1;
        for (size_t k = 0; k < t; k++)
            root_deg += codes[id][k].size(); 
        root_deg *= NI.GetDeg();

        if (root_deg > max_deg) {
            bool flag = false;
            for(size_t k = 0; k < root_id.size(); ++k) {
                if (id == root_id[k]) 
                    flag = true;
            }
            if (!flag) {
                max_deg = root_deg;
                next_root_id = id;
            }
        }
    } 
    return next_root_id;
}

template <typename id_type, typename dist_type>
void ds_cent<id_type, dist_type>::
bfs(size_t t, id_type rid){
    map<id_type, long> info_rank;

    for (TGRAPH_TYPE::TNodeI NI = net->BegNI(); NI< net->EndNI(); NI++) {
        info_rank[NI.GetId()] = -1;
    }

    queue<id_type> fifo;
    fifo.push(rid);
    codes[rid][t].push_back(rid);
    info_rank[rid] = 0;
    while(!fifo.empty()) {
        id_type pid = fifo.front();
        fifo.pop();
        for (size_t i = 0; i < net->GetNI(pid).GetDeg(); ++i) {
            id_type nid = net->GetNI(pid).GetNbrNId(i);

            if (codes[nid][t].size() != 0 &&
                    codes[nid][t].size() < codes[pid][t].size()+1)
                continue;

            long rank = rand() % 1000;
            if (info_rank[nid] < rank) {
                if (info_rank[nid] == -1) 
                    fifo.push(nid);

                codes[nid][t] = codes[pid][t];
                codes[nid][t].push_back(nid);
                info_rank[nid] = rank;
            }
        }
    }
}


template <typename id_type, typename dist_type>
void ds_cent<id_type, dist_type>::build_code_sys() {
    clock_t start_tick = clock();

    for (size_t t = 0; t < num_tree; t++) {
        cout << "processing " << t+1 << " tree" << endl;
        root_id[t] = select_root(t);
        bfs(t, root_id[t]);
    }

    out << "Building coding system time: " << 
        clock() - start_tick << "\n" << endl;
}

template <typename id_type, typename dist_type>
dist_type ds_cent<id_type, dist_type>::do_search_all(id_type src, id_type dst) {
    set<id_type> cur_set, next_set;
    cur_set.insert(src);
    dist_type est_dist = 0, min_dist = -1; // max dist
    while (*(cur_set.begin()) != dst) {
        next_set.clear();
        for (typename set<id_type>::iterator iter = cur_set.begin(); 
                iter != cur_set.end(); ++iter) {
            TGRAPH_TYPE::TNodeI cur_vertex = net->GetNI(*iter);
            for (size_t i = 0; i < cur_vertex.GetDeg(); ++ i) {
                id_type lca = -1;
                dist_type dist = get_dist(cur_vertex.GetNbrNId(i), dst, lca);
                if (dist < min_dist) {
                    min_dist = dist;
                    next_set.clear();
                    next_set.insert(cur_vertex.GetNbrNId(i));
                }
                if (dist == min_dist) {
                    next_set.insert(cur_vertex.GetNbrNId(i));
                }
            }
        }
        cur_set = next_set;
            
        // if we will reach dst code
        for (size_t i = 0; i < codes[dst].size(); ++i) {
            for (size_t j = 0; j < codes[dst][i].size(); ++j) {
                if (cur_set.find(codes[dst][i][j]) != cur_set.end()) {
                    est_dist += codes[dst][i].size() - j;
                    return est_dist; 
                }
            }
        }

        est_dist ++;
    }

    return est_dist; // should never reach here
}
template <typename id_type, typename dist_type>
dist_type ds_cent<id_type, dist_type>::do_search_multi(id_type src, id_type dst) {
    set<id_type> cur_set, next_set;
    map< id_type, pair<size_t, id_type> > lca_vertex;
    cur_set.insert(src);
    dist_type est_dist = 0, min_dist = -1; // max dist
    
    while (*(cur_set.begin()) != dst) {
        next_set.clear();
        for (typename set<id_type>::iterator iter = cur_set.begin(); 
                iter != cur_set.end(); ++iter) {
            TGRAPH_TYPE::TNodeI cur_vertex = net->GetNI(*iter);
            for (size_t i = 0; i < cur_vertex.GetDeg(); ++ i) {
                id_type lca = -1;
                dist_type dist = get_dist(cur_vertex.GetNbrNId(i), dst, lca);
                if (dist < min_dist) {
                    min_dist = dist;
                    next_set.clear();
                    next_set.insert(cur_vertex.GetNbrNId(i));
                    lca_vertex.clear();
                    size_t deg = net->GetNI(cur_vertex.GetNbrNId(i)).GetDeg();
                    lca_vertex[lca] = make_pair(deg, cur_vertex.GetNbrNId(i));
                }
                if (dist == min_dist) {
                    size_t deg = net->GetNI(cur_vertex.GetNbrNId(i)).GetDeg();
                    if (lca_vertex.find(lca) == lca_vertex.end()) {
                        lca_vertex[lca] = make_pair(deg, cur_vertex.GetNbrNId(i));
                    }
                    else {
                        if (deg > lca_vertex[lca].second) {
                            lca_vertex[lca] = make_pair(deg, cur_vertex.GetNbrNId(i));
                        }
                    }
                }
            }
        }
        for (typename map< id_type, pair<size_t, id_type> >::iterator 
                iter = lca_vertex.begin();
                iter != lca_vertex.end(); ++iter)
            cur_set.insert(iter->second.second);
            
        // if we will reach dst code
        for (size_t i = 0; i < codes[dst].size(); ++i) {
            for (size_t j = 0; j < codes[dst][i].size(); ++j) {
                if (cur_set.find(codes[dst][i][j]) != cur_set.end()) {
                    est_dist += codes[dst][i].size() - j;
                    return est_dist; 
                }
            }
        }

        est_dist ++;
    }

    return est_dist; // should never reach here
}

template <typename id_type, typename dist_type>
dist_type ds_cent<id_type, dist_type>::do_search(id_type src, id_type dst) {
    id_type cur = src, next = src;
    dist_type est_dist = 0, min_dist = -1; // max dist
    while (cur != dst) {
        TGRAPH_TYPE::TNodeI cur_vertex = net->GetNI(cur);
        for (size_t i = 0; i < cur_vertex.GetDeg(); ++ i) {
            id_type lca = -1;
            dist_type dist = get_dist(cur_vertex.GetNbrNId(i), dst, lca);
            if (dist < min_dist) {
                min_dist = dist;
                next = cur_vertex.GetNbrNId(i);
            }
            else if (dist == min_dist) {
                if (rand() % 10 > 5) {
                    next = cur_vertex.GetNbrNId(i);
                }
            }
        }
        cur = next;
        // if we will reach dst code
        for (size_t i = 0; i < codes[dst].size(); i++) {
            for (size_t j = 0; j < codes[dst][i].size(); j++) {
                if (cur == codes[dst][i][j]) {
                    est_dist += codes[dst][i].size() - j;
                    return est_dist; 
                }
            }
        }
        est_dist ++;
    }

    return est_dist; // should never reach here
}

template <typename id_type, typename dist_type>
vector< pair<id_type, dist_type> > ds_cent<id_type, dist_type>::
get_bfs_order(vector< vector<id_type> > sketch) {
    vector<id_type> bfs_nodes;
    vector< pair<id_type, dist_type> > bfs_of_sketch;

    for (size_t t = 0; t < sketch.size(); t++) {
        reverse(sketch[t].begin(), sketch[t].end());
    }
    size_t step = 0;
    bool flag = true;
    while(flag) {
        flag = false;
        for (size_t t = 0; t < sketch.size(); t++) {
            if (step < sketch[t].size()) {
                flag = true;
                if (find(bfs_nodes.begin(), bfs_nodes.end(), 
                            sketch[t][step]) == bfs_nodes.end()) {
                    bfs_nodes.push_back(sketch[t][step]);
                    bfs_of_sketch.push_back(make_pair(sketch[t][step], step));
                }
            }
        }
        step ++;
    }

    return bfs_of_sketch;
}

template <typename id_type, typename dist_type>
dist_type ds_cent<id_type, dist_type>::tree_sketch(id_type src, id_type dst) {
    vector< pair<id_type, dist_type> > bfs_src;
    vector< pair<id_type, dist_type> > bfs_dst;
    dist_type l_shortest = -1;
    vector< pair<id_type, dist_type> > v_src;
    vector< pair<id_type, dist_type> > v_dst;

    bfs_src = get_bfs_order(codes[src]);
    bfs_dst = get_bfs_order(codes[dst]);

    size_t length = max(bfs_src.size(), bfs_dst.size());
    for (size_t step = 0; step < length; step ++) {
        id_type u = -1;
        id_type v = -1;
        dist_type d_su = -1;
        dist_type d_dv = -1;
        if (step < bfs_src.size()) {
            u = bfs_src[step].first;
            d_su = bfs_src[step].second;
            v_src.push_back(bfs_src[step]);
        }
        if (step < bfs_dst.size()) {
            v = bfs_dst[step].first;
            d_dv = bfs_dst[step].second;
            v_dst.push_back(bfs_dst[step]);
        }

        if (v != -1) {
            for (size_t i = 0; i < v_src.size(); i++) {
                if (net->IsEdge(v_src[i].first, v) or net->IsEdge(v, v_src[i].first)) {
                    dist_type l = v_src[i].second + d_dv + 1;
                    if (l < l_shortest)
                        l_shortest = l;
                }
            }
        }

        if (u != -1) {
            for (size_t i = 0; i < v_dst.size(); i++) {
                if (net->IsEdge(v_dst[i].first, u) or net->IsEdge(u, v_dst[i].first)) {
                    dist_type l = v_dst[i].second + d_su + 1;
                    if (l < l_shortest)
                        l_shortest = l;
                }
            }
        }

        if (d_su + d_dv >= l_shortest)
            break;
    }

    return l_shortest;
}

template <typename id_type, typename dist_type>
void ds_cent<id_type, dist_type>::test() {
    string tcfilename = "./datasets/testcases/withreal/";
    tcfilename += graphname + "_testcases.txt";
    cout << tcfilename << endl;
    ifstream in(tcfilename.c_str());
    TBreathFS<PGRAPH_TYPE> BFS(net);
    size_t cnt = 0;

    while (cnt < num_exp) {
        if (cnt % 1000 == 999)
            cout << "." << flush;
        id_type src;
        id_type dst;
        dist_type real_dist;
        in >> src >> dst >> real_dist;
        total_real += real_dist;
        cnt ++;

        id_type lca = -1;
        dist_type obv_dist = get_dist(src, dst, lca);
        total_obv += double(obv_dist - real_dist) / real_dist;

        dist_type comp_dist = tree_sketch(src, dst);
        total_comp += double(comp_dist - real_dist) / real_dist;

        dist_type est_dist_1, est_dist_2, est_dist;
        /*
        est_dist_1 = do_search(src, dst);
        est_dist_2 = do_search(dst, src);
        est_dist = est_dist_1 < est_dist_2 ? est_dist_1 : est_dist_2;
        total_est += double(est_dist - real_dist) / real_dist;
        */

        est_dist_1 = do_search_multi(src, dst);
        est_dist_2 = do_search_multi(dst, src);
        est_dist = est_dist_1 < est_dist_2 ? est_dist_1 : est_dist_2;
        total_est_multi += double(est_dist - real_dist) / real_dist;
        
        /*
        est_dist_1 = do_search_all(src, dst);
        est_dist_2 = do_search_all(dst, src);
        est_dist = est_dist_1 < est_dist_2 ? est_dist_1 : est_dist_2;
        total_est_all += double(est_dist - real_dist) / real_dist;
        */
    }
    in.close();
    cout << endl;
}

template <typename id_type, typename dist_type>
void ds_cent<id_type, dist_type>::print_info(int stage) {
    switch (stage) {
        case 1:
            for (size_t t = 0; t  < num_tree; t ++) {
                out << "Set" << t << ": " << endl;
                out << "Root ID: " << root_id[t] << endl;
                out << "Root Deg: " << 
                    net->GetNI(root_id[t]).GetDeg() << endl;
            }
            break;
        case 2:
            out << "Number of experiments: " << num_exp << endl;
            out << "Avg real: " << double(total_real) / num_exp << endl;
            out << "Avg est all: " << total_est_all / num_exp << endl;
            out << "Avg est multi: " << total_est_multi / num_exp << endl;
            out << "Avg est: " << total_est / num_exp << endl;
            out << "Avg comp: " << total_comp / num_exp << endl;
            out << "Avg obv: " << total_obv / num_exp << endl;
            out << endl;
            break;
        default:
            break;
    }
}

int main(int argc, char** argv){
    cerr << "Step 1: Setting Environment and loading graphs." << endl;
    string graphfile;
    if (argc < 2){
        cout << "Please input graph file name: " << endl;
        cin >> graphfile;
    }
    else {
        graphfile = argv[1];
    }
    ds_cent<unsigned long, unsigned long> m(graphfile);

    cerr << "Step 2: Building code system." << endl;
    m.build_code_sys();
    m.print_info(1);

    cerr << "Step 3: Test coordinate system with simple search." << endl;
    m.test();
    m.print_info(2);

    cerr << "Done." << endl;
    return 0;
}

