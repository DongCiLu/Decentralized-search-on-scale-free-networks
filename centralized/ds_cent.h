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

        ds_cent(std::string graphfile, size_t n_tree);
        ~ds_cent();
        void build_index(size_t t);
        void test();
        void print_info(int stage);
        void reset();

    private:
        dist_type get_dist(id_type src_id, id_type dst_id, id_type& lca);
        dist_type do_search(id_type src, id_type dst); 
        dist_type do_search_multi(id_type src, id_type dst); 
        dist_type do_search_all(id_type src, id_type dst, 
                std::set< std::vector<id_type> > &pair_path); 
        std::vector< std::pair<id_type, dist_type> > 
            get_bfs_order(std::vector< std::vector<id_type> > sketch);
        dist_type tree_sketch(
                id_type src, id_type dst, size_t &path_cnt);
        id_type select_root(size_t t);

        void bfs(size_t t, id_type rid);

        std::string get_time();

        std::string graphname;

        PGRAPH_TYPE net;

        code_type codes;
        std::ofstream out;
        std::vector<id_type> root_id;

        size_t total_path_cnt;
        size_t total_comp_path_cnt;
        double total_out_ratio;

        dist_type total_real;
        double total_est_all;
        double total_est_multi;
        double total_est;
        double total_comp;
        double total_obv;

        size_t index_oh;

        size_t n_tree;
        size_t n_exp;
        dist_type max_dist;

        std::map<id_type, long> unlabeled_degree;

};

#endif
