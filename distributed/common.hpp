#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <set>
#include <map>
#include <iterator>

#include <boost/thread/mutex.hpp>

#include "graphlab.hpp"

#define EARLY_TERMINATION //Never turn off
#define BIDIRECTIONAL_SEARCH
//#define TIE_FULL
//#define LABEL_DEG
//#define SELECTIVE_LCA
//#define TIE_HEUR

//#define CALC_REAL //Turn off all the others when turn this on

/******************* Declear basic types for graph *******************/
bool DIRECTED_GRAPH = false;

typedef unsigned long distance_type; // no support for float point weights
typedef graphlab::vertex_id_type code_type;
typedef std::vector< std::vector<code_type> > label_type;

struct vertex_data {
    label_type code;
#ifdef CALC_REAL
    size_t flag;
#endif
    size_t check_cnt;
#ifdef SELECTIVE_LCA
    size_t ignore_cnt;
#endif

    vertex_data() {
#ifdef CALC_REAL
        flag = 0;
#endif
        check_cnt = 0;
#ifdef SELECTIVE_LCA
        ignore_cnt = 0;
#endif
    }

    void save(graphlab::oarchive& oarc) const {
        oarc << code;
#ifdef CALC_REAL
        oarc << flag;
#endif
        oarc << check_cnt;
#ifdef SELECTIVE_LCA
        oarc << ignore_cnt;
#endif
    }

    void load(graphlab::iarchive& iarc) {
        iarc >> code;
#ifdef CALC_REAL
        iarc >> flag;
#endif
        iarc >> check_cnt;
#ifdef SELECTIVE_LCA
        iarc >> ignore_cnt;
#endif
    }
}; // end of vertex data

struct edge_data : graphlab::IS_POD_TYPE {
    distance_type dist;
    edge_data(distance_type dist = 1) : dist(dist) { }
}; // end of edge data

typedef graphlab::distributed_graph<vertex_data, edge_data> graph_type;

inline graph_type::vertex_type
get_other_vertex(const graph_type::edge_type& edge,
        const graph_type::vertex_type& vertex) {
    return vertex.id() == edge.source().id()? edge.target() : edge.source();
}

/******************* For Greedy search *******************/
enum {Invalid, Started, Main, Finished, Failed, Finalized};

struct gsInstance {
    // searching related members
    size_t id;
    distance_type min_dist;
    distance_type real_dist;
    int state;
    size_t tie_cnt;
    graphlab::vertex_id_type src_id;
    graphlab::vertex_id_type dst_id;
    label_type src_code;
    label_type dst_code;
    std::vector<code_type> path;

    gsInstance() : id(std::numeric_limits<size_t>::max()), 
                   min_dist(std::numeric_limits<distance_type>::max()), 
                   real_dist(std::numeric_limits<distance_type>::max()), 
                   state(Invalid), tie_cnt(1) { }

    void save(graphlab::oarchive& oarc) const {
        oarc << id << min_dist << real_dist << state << tie_cnt << 
            src_id << dst_id << src_code << dst_code << path;
    }

    void load(graphlab::iarchive& iarc) {
        iarc >> id >> min_dist >> real_dist >> state >> tie_cnt >>
            src_id >> dst_id >> src_code >> dst_code >> path;
    }
};

inline distance_type get_code_dist(const label_type& src_code, 
        const label_type& dst_code) {
    distance_type dist = std::numeric_limits<distance_type>::max();
    for (size_t t = 0; t < src_code.size(); t++){
        size_t range = std::min(src_code[t].size(), dst_code[t].size());
        size_t i = 0;
        while (i < range) { // we use while because we need i later
            if (src_code[t][i] != dst_code[t][i])
                break;
            i++;
        }
        if (i != 0)
            dist = std::min(dist, src_code[t].size() 
                    + dst_code[t].size() - 2 * i);
    }
    return dist;
}

/******************* Global Variables *******************/
std::vector< std::map<size_t, gsInstance> > results;
#ifdef CALC_REAL
typedef typename std::multimap<graphlab::vertex_id_type, size_t> bfs_ds_type;
bfs_ds_type bfs_dst_set;
std::vector< std::map<size_t, distance_type> > real_results;
#endif
boost::mutex mtx;
size_t procid;

/******************* Others *******************/
struct select_root_reducer: public graphlab::IS_POD_TYPE {
    size_t rank;
    graphlab::vertex_id_type vid;
    select_root_reducer& operator+=(const select_root_reducer& other) {
        if (rank < other.rank) {
            (*this) = other;
        }
        return (*this);
    }
};

select_root_reducer calc_root_rank(const graph_type::vertex_type vtx) {
    select_root_reducer red;
    red.rank = 1;
    for (size_t i = 0; i < vtx.data().code.size(); i++){
        red.rank += vtx.data().code[i].size() - 1;
        if (vtx.data().code[i].size()-1 == 0) {
            red.rank = 0;
            break;
        }
    }
    red.rank *= vtx.num_in_edges() + vtx.num_out_edges();
    if (vtx.data().code.size() > 0) red.rank /= vtx.data().code.size();
    red.vid = vtx.id();
    return red;
}

struct calc_overhead_reducer {
    size_t code_overhead;
    size_t search_overhead;
#ifdef SELECTIVE_LCA
    size_t reduced_overhead;
#endif

    calc_overhead_reducer& operator+=(const calc_overhead_reducer& other) {
        code_overhead += other.code_overhead;
        search_overhead += other.search_overhead;
#ifdef SELECTIVE_LCA
        reduced_overhead += other.reduced_overhead;
#endif
        return (*this);
    }

    void save(graphlab::oarchive& oarc) const {
        oarc << code_overhead << search_overhead;
#ifdef SELECTIVE_LCA
        oarc << reduced_overhead;
#endif
    }

    void load(graphlab::iarchive& iarc) {
        iarc >> code_overhead >> search_overhead;
#ifdef SELECTIVE_LCA
        iarc >> reduced_overhead;
#endif
    }
};

calc_overhead_reducer calc_overhead(const graph_type::vertex_type vtx) {
    calc_overhead_reducer red;
    red.code_overhead = 0;
    red.search_overhead = vtx.data().check_cnt;
#ifdef SELECTIVE_LCA
    red.reduced_overhead = vtx.data().ignore_cnt;
#endif
    for (size_t i = 0; i < vtx.data().code.size(); i++) 
        red.code_overhead += vtx.data().code[i].size();
    return red;
}

#endif
