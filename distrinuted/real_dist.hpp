#ifndef REAL_DIST_H
#define REAL_DIST_H

#include "common.hpp"

struct real_min_distance_type : public graphlab::IS_POD_TYPE {
    distance_type dist;

    real_min_distance_type(distance_type dist = 
            std::numeric_limits<distance_type>::max()
            ) : dist(dist) { } 

    real_min_distance_type& operator+=(
            const real_min_distance_type& other) {
        dist = std::min(dist, other.dist);
        return *this;
    }
};

class real_sssp :
    public graphlab::ivertex_program<graph_type, 
    graphlab::empty,
    real_min_distance_type>,
    public graphlab::IS_POD_TYPE{
        distance_type min_dist;
        public:

            void init(icontext_type& context, const vertex_type& vertex,
                    const real_min_distance_type& msg) {
                min_dist = msg.dist;
            } 

            edge_dir_type gather_edges(icontext_type& context, 
                    const vertex_type& vertex) const { 
                return graphlab::NO_EDGES;
            } // end of gather_edges 


            void apply(icontext_type& context, vertex_type& vertex,
                    const graphlab::empty& empty) {
                vertex.data().flag++;
                vertex_id_type vid = vertex.id();
                std::pair< bfs_ds_type::iterator, bfs_ds_type::iterator>
                    piters = bfs_dst_set.equal_range(vid);
                mtx.lock();
                for (bfs_ds_type::iterator iter = piters.first;
                        iter != piters.second; ++iter){
                    real_results[procid][iter->second] = min_dist;
                }
                mtx.unlock();
                min_dist ++;
            }

            edge_dir_type scatter_edges(icontext_type& context, 
                    const vertex_type& vertex) const {
                return DIRECTED_GRAPH? graphlab::OUT_EDGES : 
                    graphlab::ALL_EDGES; 
            } // end of scatter_edges

            void scatter(icontext_type& context, const vertex_type& vertex,
                    edge_type& edge) const {
                const vertex_type other = get_other_vertex(edge, vertex);
                if (other.data().flag < vertex.data().flag) {
                    context.signal(other,real_min_distance_type(min_dist));
                }
            } // end of scatter
    };
#endif
