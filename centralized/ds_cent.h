/* 
 * Author: Zheng Lu
 * Date: Jul. 15, 2014
 * Description: Maximum information multitree for code system.
 */

#ifndef DS_CENT_H
#define DS_CENT_H

#define TGRAPH_TYPE TNGraph
#define PGRAPH_TYPE PNGraph

#include <string>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <cstdlib>

#include "Snap.h"

template <typename id_type>
struct temp_data {
    size_t level;
    std::vector<id_type> parents;
};

template <typename id_type = unsigned long, typename dist_type = unsigned long>
class ds_cent {
    public:
        typedef std::map< id_type, std::vector< std::vector<id_type> > > 
            code_type;

        ds_cent(std::string graphfile);
        ~ds_cent();
        void build_code_sys(); // build code system
        void test(); // performing decentralized search
        void print_info(int stage);

    private:
        dist_type get_dist(id_type src_id, id_type dst_id, id_type& lca);
        dist_type do_search(id_type src, id_type dst); 
        dist_type do_search_multi(id_type src, id_type dst); 
        dist_type do_search_all(id_type src, id_type dst); 
        std::vector< std::pair<id_type, dist_type> > 
            get_bfs_order(std::vector< std::vector<id_type> > sketch);
        dist_type tree_sketch(id_type src, id_type dst);
        id_type select_root(size_t t);

        void bfs(size_t t, id_type rid);

        std::string get_time();

        std::string graphname;

        PGRAPH_TYPE net;

        code_type codes;
        std::ofstream out;
        std::vector<id_type> root_id;

        clock_t total_tick;
        clock_t total_comp_tick;
        size_t total_path_cnt;

        dist_type total_real;
        dist_type total_est_all;
        dist_type total_est_multi;
        dist_type total_est;
        dist_type total_comp;
        dist_type total_obv;

        const size_t num_tree;
        size_t num_exp;
        dist_type max_dist;

        std::map<id_type, long> unlabeled_degree;

};

#endif
