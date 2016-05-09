#ifndef BUILD_TREE_H
#define BUILD_TREE_H

#include "common.hpp"

struct min_distance_type {
    size_t tree_id; /*for multi-tree*/
    long rank;
    long potential;
    std::vector<code_type> code;
#ifdef REDUCE_RED_LABEL
    size_t red_cnt;
#endif

#ifdef REDUCE_RED_LABEL
    min_distance_type(size_t tree_id = 
            std::numeric_limits<size_t>::max(),
            long rank = -1, size_t red_cnt = 10000) : 
        tree_id(tree_id), rank(rank), potential(rank), red_cnt(red_cnt) {}
#else
    min_distance_type(size_t tree_id = 
            std::numeric_limits<size_t>::max(),
            long rank = -1) : 
        tree_id(tree_id), rank(rank), potential(rank) {}
#endif

#ifdef REDUCE_RED_LABEL
    min_distance_type(size_t tree_id, long rank, 
            const std::vector<code_type>& code, 
            size_t red_cnt) :
        tree_id(tree_id), rank(rank), potential(rank), 
        code(code), red_cnt(red_cnt) {} 
#else
    min_distance_type(size_t tree_id, long rank, 
            const std::vector<code_type>& code) :
        tree_id(tree_id), rank(rank), potential(rank), code(code) {} 
#endif

    min_distance_type& operator+=(const min_distance_type& other) {
#ifdef LABEL_DEG
        long this_rank = rank;
        long other_rank = other.rank;
#else
        long this_rank = rand() % 10000;
        long other_rank = rand() % 10000;
#endif

#ifdef REDUCE_RED_LABEL
        if (red_cnt > 0)
            this_rank = 0;
        if (other.red_cnt > 0)
            other_rank = 0;
#endif
        if (this_rank < other_rank) {
            code = other.code;
            rank = other.rank;
        }
        if (this_rank == other_rank) {
            if (rand() % 10000 > 5000){
                code = other.code;
                rank = other.rank;
            }
        }
        potential += other.potential;
        return *this;
    }

    void save(graphlab::oarchive& oarc) const {
        oarc << tree_id << rank << potential << code;
    }

    void load(graphlab::iarchive& iarc) {
        iarc >> tree_id >> rank >> potential >> code;
    }
};

class build_code_sys :
    public graphlab::ivertex_program<graph_type, 
    graphlab::empty,
    min_distance_type> {
        size_t tree_id;
        std::vector<code_type> code;
        long rank;
        long potential;
        bool changed; 

        public:
            void init(icontext_type& context, const vertex_type& vertex,
                    const min_distance_type& msg) {
                tree_id = msg.tree_id;
                code = msg.code;
                rank = msg.rank;
                potential = msg.potential;
                changed = false;
            } 

            edge_dir_type gather_edges(icontext_type& context, 
                    const vertex_type& vertex) const { 
                return graphlab::NO_EDGES;
            } // end of gather_edges 

            void apply(icontext_type& context, vertex_type& vertex,
                    const graphlab::empty& empty) {
                if (vertex.data().code.size() == tree_id) {
                    changed = true;
                    vertex.data().code.push_back(code);
                    vertex.data().code[tree_id].push_back(vertex.id());
                    std::vector<code_type>(vertex.data().code[tree_id].begin(),
                            vertex.data().code[tree_id].end()).swap(
                            vertex.data().code[tree_id]);
                    label_type(vertex.data().code.begin(), 
                            vertex.data().code.end()).swap(vertex.data().code);
                }
            }

            edge_dir_type scatter_edges(icontext_type& context, 
                    const vertex_type& vertex) const {
                if(changed)
                    return DIRECTED_GRAPH? graphlab::OUT_EDGES : 
                        graphlab::ALL_EDGES; 
                else return graphlab::NO_EDGES;
            } // end of scatter_edges

            void scatter(icontext_type& context, const vertex_type& vertex,
                    edge_type& edge) const {
                const vertex_type other = get_other_vertex(edge, vertex);
                if (other.data().code.size() == tree_id) {
#ifdef REDUCE_RED_LABEL
                    size_t red_cnt = 0;
                    for (size_t t = 0; t < other.data().code.size(); t++) {
                        size_t len = other.data().code[t].size();
                        if (len > 1 && 
                                vertex.id() == other.data().code[t][len - 2])
                            red_cnt ++;
                    }
#endif
                    long new_r = rank + 
                        vertex.num_in_edges() + 
                        vertex.num_out_edges();
                    const min_distance_type 
#ifdef REDUCE_RED_LABEL
                        msg(tree_id, new_r, vertex.data().code[tree_id], red_cnt);
#else
                        msg(tree_id, new_r, vertex.data().code[tree_id]);
#endif
                    context.signal(other, msg);
                }
            } // end of scatter

            void save(graphlab::oarchive& oarc) const {
                oarc << tree_id << code << rank << potential << changed;
            }

            void load(graphlab::iarchive& iarc) {
                iarc >> tree_id >> code >> rank >> potential >> changed;
            }

    };

#endif
