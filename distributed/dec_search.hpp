#ifndef DEC_SEARCH_H
#define DEC_SEARCH_H

#include "common.hpp"

struct hop_msg_type {
    std::vector<gsInstance> inst_set; // instance id to instance

    hop_msg_type() { }

    hop_msg_type(const gsInstance& inst) { 
        inst_set.push_back(inst); 
    }

    hop_msg_type& operator+=(const hop_msg_type& other) {
#ifdef TIE_FULL
        // there could be overlaps
        for (std::vector<gsInstance>::const_iterator 
                iter = other.inst_set.begin();
                iter != other.inst_set.end(); ++iter) {
            bool found = false;
            for (size_t i = 0; i < inst_set.size(); ++i) {
                if(inst_set[i].id == iter->id) {
                    found = true;
                    if (iter->state == Main)
                        inst_set[i].state = Main;
                }
            }
            if (!found)
                inst_set.push_back(*iter);
        }
#else
        // only one node per hop, no overlaps
        inst_set.insert(inst_set.end(), 
                other.inst_set.begin(), other.inst_set.end());
#endif //TIE_FULL
        return *this;
    }

    void save(graphlab::oarchive& oarc) const {
        oarc << inst_set;
    }

    void load(graphlab::iarchive& iarc) {
        iarc >> inst_set;
    }

};

#ifdef TIE_FULL
struct mc_instance{
    size_t id;
    distance_type dist;
#ifdef TIE_HEUR
    std::map<graphlab::vertex_id_type, 
        graphlab::vertex_id_type> vids;
#else
    std::set<graphlab::vertex_id_type> vids;
#endif

    mc_instance(size_t id = -1, distance_type dist = 
            std::numeric_limits<distance_type>::max()):
        id(id), dist(dist) { }

#ifdef TIE_HEUR
    mc_instance(size_t id, distance_type dist, 
            std::map<graphlab::vertex_id_type, 
            graphlab::vertex_id_type>& vids):
#else
    mc_instance(size_t id, distance_type dist, 
            std::set<graphlab::vertex_id_type>& vids):
#endif
        id(id), dist(dist), vids(vids) { }

    void save(graphlab::oarchive& oarc) const {
        oarc << id << dist << vids;
    }

    void load(graphlab::iarchive& iarc) {
        iarc >> id >> dist >> vids;
    }
};
#else
struct mc_instance : graphlab::IS_POD_TYPE {
    size_t id;
    distance_type dist;
    graphlab::vertex_id_type vid;
    long potential;

    mc_instance(size_t id = -1,
            distance_type dist = std::numeric_limits<distance_type>::max(),
            graphlab::vertex_id_type vid = -1, 
            long potential = 0):
        id(id), dist(dist), vid(vid), potential(potential) { }
};
#endif //TIE_FULL

// Gather type for greedy search
struct min_code_distance_type {
    std::vector<mc_instance> mc_inst_set;

    min_code_distance_type() { } 

    // create an gather instance from one point to all gsInstance
    min_code_distance_type(graphlab::vertex_id_type vid, 
            const label_type& vcode, 
            const std::vector<gsInstance>& inst_set) {
#ifdef DEBUG_STEP_TIMER
        clock_t timer = clock();
#endif

#ifdef TIE_FULL
#ifdef TIE_HEUR
        std::map<graphlab::vertex_id_type, graphlab::vertex_id_type> 
            vids;
#else
        std::set<graphlab::vertex_id_type> vids;
        vids.insert(vid);
#endif
#endif //TIE_FULL
        mc_inst_set.resize(inst_set.size());
        size_t mc_cnt = 0;
        for(std::vector<gsInstance>::const_iterator 
                iter = inst_set.begin();
                iter != inst_set.end(); ++iter) {
#ifdef TIE_HEUR
            graphlab::vertex_id_type lca = -1;
            distance_type dist = 
                get_code_dist_wlca(vcode, iter->dst_code, lca);
            vids.clear();
            vids[lca] = vid;
#else
            distance_type dist = 
                get_code_dist(vcode, iter->dst_code);
#endif

            mc_inst_set[mc_cnt].id = iter->id;
            mc_inst_set[mc_cnt].dist = dist;
#ifdef TIE_FULL
            mc_inst_set[mc_cnt].vids.swap(vids);
#else
            mc_inst_set[mc_cnt].vid = vid;

#endif // TIE_FULL
            //mc_inst_set.push_back(mcInst);
            //mc_inst_set[mc_cnt] = mcInst;
            mc_cnt ++;
        }

#ifdef DEBUG_STEP_TIMER
        timer = clock() - timer;
        step_mtx.lock();
        agg_step_timer[4] += timer;
        step_mtx.unlock();
#endif
    } 

    // combine gather instance from all neighbors
    min_code_distance_type& operator+=(
            const min_code_distance_type& other) {
#ifdef DEBUG_STEP_TIMER
        clock_t timer = clock();
#endif
        size_t pos = 0;
        for(std::vector<mc_instance>::const_iterator 
                iter = other.mc_inst_set.begin(); 
                iter != other.mc_inst_set.end(); ++iter) {
            // both vector generate from same set so they are in same order
            if (iter->dist < mc_inst_set[pos].dist) 
                mc_inst_set[pos] = *iter;
#ifdef TIE_FULL
            else if (iter->dist == mc_inst_set[pos].dist) {
                mc_inst_set[pos].vids.insert(iter->vids.begin(), 
                        iter->vids.end());
            }
#endif //TIE_FULL
            pos ++;
        }
#ifdef DEBUG_STEP_TIMER
        timer = clock() - timer;
        step_mtx.lock();
        agg_step_timer[5] += timer;
        step_mtx.unlock();
#endif
        return *this;
    }

    void save(graphlab::oarchive& oarc) const {
        oarc << mc_inst_set;
    }

    void load(graphlab::iarchive& iarc) {
        iarc >> mc_inst_set;
    }

};

class dec_search :
    public graphlab::ivertex_program<graph_type, 
    min_code_distance_type,
    hop_msg_type> {
        // support query level parallelism
        // using vector instead of set to gurantee the sequance 
        std::vector<gsInstance> inst_set;
#ifdef TIE_FULL
        std::vector< std::set<graphlab::vertex_id_type> > next_hop_set;
        std::vector<graphlab::vertex_id_type> main_search_vid;
#endif //TIE_FULL

        public:

        void init(icontext_type& context, const vertex_type& vertex,
                const hop_msg_type& msg) {
            inst_set = msg.inst_set;
#ifdef DEBUG_STEP_TIMER
            step_mtx.lock();
            if (step_cnt < inst_set.begin()->path.size()) {
                if (last_step != 0)
                    agg_step_timer[0] += clock() - last_step;
                last_step = clock();
                step_cnt ++;
            }
            step_mtx.unlock();
#endif
        } 

        edge_dir_type gather_edges(icontext_type& context, 
                const vertex_type& vertex) const { 
            return DIRECTED_GRAPH? graphlab::OUT_EDGES : 
                graphlab::ALL_EDGES; 
        } // end of gather_edges 


        min_code_distance_type gather(icontext_type& context, 
                const vertex_type& vertex, 
                edge_type& edge) const {
#ifdef DEBUG_STEP_TIMER
            clock_t gather_timer = clock();
#endif
            vertex_type other = get_other_vertex(edge, vertex);
#ifdef SELECTIVE_LCA
            size_t ignore_tree_cnt = 0;
            for (size_t i = 0; i < other.data().code.size(); i++) {
                size_t len = other.data().code[i].size();
                if (len > 1 && vertex.id() == other.data().code[i][len - 2])
                    ignore_tree_cnt ++;
            }
            //other.data().ignore_cnt += 
            //    inst_set.size() * ignore_tree_cnt;
#endif
            //other.data().check_cnt += 
            //    inst_set.size() * other.data().code.size();
            other.data().check_cnt += inst_set.size();
#ifdef DEBUG_STEP_TIMER
            gather_timer = clock() - gather_timer;
            step_mtx.lock();
            agg_step_timer[3] += gather_timer;
            step_mtx.unlock();
#endif
            return min_code_distance_type(other.id(), 
                    other.data().code, inst_set);
        } // end of gather function

        void store_result(std::vector<gsInstance>::iterator iter) {
            mtx.lock();
#ifdef TIE_FULL
            if (results[procid].find(iter->id) != results[procid].end() && 
                    results[procid][iter->id].path.size() ==
                    iter->path.size()) 
                results[procid][iter->id].tie_cnt ++;
            if (results[procid].find(iter->id) == results[procid].end() || 
                    results[procid][iter->id].path.size() > 
                    iter->path.size()) 
                results[procid][iter->id] = *iter;
#else
            results[procid].insert(std::make_pair(iter->id, *iter));
#endif //TIE_FULL
            mtx.unlock();
        }

        void apply(icontext_type& context, vertex_type& vertex,
                const min_code_distance_type& min_code_dist) {
#ifdef DEBUG_STEP_TIMER
            clock_t timer = clock();
            clock_t timer1_sum = 0;
            clock_t timer2_sum = 0;
            clock_t timer3_sum = 0;
#endif
            vertex.data().visit_cnt += min_code_dist.mc_inst_set.size();
            std::vector<mc_instance>::const_iterator mcIter = 
                min_code_dist.mc_inst_set.begin();
            std::vector<gsInstance>::iterator iter = inst_set.begin();

            // mc_inst_set and inst_set are in same order and same size
            while(mcIter != min_code_dist.mc_inst_set.end()) {
#ifdef TIE_FULL
#ifdef TIE_HEUR
                std::set<graphlab::vertex_id_type> vids;
                for (typename std::map<graphlab::vertex_id_type, 
                        graphlab::vertex_id_type>::const_iterator convert_iter =
                        mcIter->vids.begin();
                        convert_iter != mcIter->vids.end();
                        ++ convert_iter)
                    vids.insert(convert_iter->second);
#else
                std::set<graphlab::vertex_id_type> vids = mcIter->vids;
#endif
#endif

#ifdef DEBUG_STEP_TIMER
                clock_t timer1 = clock();
#endif

#ifdef EARLY_TERMINATION
                //try to find next step node in code[dst] 
                bool found = false;
                for (size_t t = 0; t < iter->dst_code.size() &&
                        !found; ++t) {
                    for (size_t i = 0; i < iter->dst_code[t].size(); ++i) {
#ifdef TIE_FULL
                        /* 
                         * this is not totally correct,
                         * but have good effort and outcome tradeoff
                         */
                        if (vids.find(iter->dst_code[t][i]) != vids.end()){
#else
                        if (mcIter->vid == iter->dst_code[t][i]){
#endif //TIE_FULL
                            found = true;
                            if (vertex.id() == iter->dst_id)
                                i = iter->dst_code[t].size();
                            for (; i < iter->dst_code[t].size(); ++i)
                                iter->path.push_back(iter->dst_code[t][i]);
                        }
                    }
                }
#endif //ET

#ifdef DEBUG_STEP_TIMER
                timer1_sum += clock() - timer1;
#endif

                // test if finished - termination condition 1
#ifdef EARLY_TERMINATION
                if(found || vertex.id() == iter->dst_id) {
#else
                if(vertex.id() == iter->dst_id) {
#endif //ET

#ifdef DEBUG_STEP_TIMER
                    clock_t timer2 = clock();
#endif
                    iter->state = Finished;
                    store_result(iter);
                    ++iter;
                    //iter = inst_set.erase(iter);
                    
#ifdef DEBUG_STEP_TIMER
                    timer2_sum += clock() - timer2;
#endif
                }
                // regular update, update record with next hop
                else if (mcIter->dist < iter->min_dist) {
#ifdef DEBUG_STEP_TIMER
                    clock_t timer3 = clock();
#endif
#ifdef TIE_FULL
                    if (iter->state != Main &&
                            iter->min_dist <= mcIter->dist + 1)
                        vids.clear();
#endif
                    iter->min_dist = mcIter->dist;
#ifdef TIE_FULL
                    if (!vids.empty()) {
                        next_hop_set.push_back(vids);
                        main_search_vid.push_back(*(vids.begin()));
                        iter ++;
                    }
                    else {
                        iter = inst_set.erase(iter);
                    }
#else
                    iter->path.push_back(mcIter->vid);
                    ++iter;
#endif //TIE_FULL
#ifdef DEBUG_STEP_TIMER
                    timer3_sum += clock() - timer3;
#endif
                }
                else {
                    std::cout << "FATAL: Not correct branch!" << std::endl;
                    ++iter;
                }
                ++mcIter;
            }

            std::vector<gsInstance> new_inst_set;
            for (std::vector<gsInstance>::iterator iter = inst_set.begin();
                    iter != inst_set.end();
                    ++ iter) {
                if (iter->state != Finished)
                    new_inst_set.push_back(*iter);
            }
            inst_set.swap(new_inst_set);


#ifdef DEBUG_STEP_TIMER
            timer = clock() - timer;
            step_mtx.lock();
            agg_step_timer[1] += timer;
            agg_step_timer[6] += timer1_sum;
            agg_step_timer[7] += timer2_sum;
            agg_step_timer[8] += timer3_sum;
            step_mtx.unlock();
#endif
        }

        edge_dir_type scatter_edges(icontext_type& context, 
                const vertex_type& vertex) const {
            if (!inst_set.empty())
                return DIRECTED_GRAPH? graphlab::OUT_EDGES : 
                    graphlab::ALL_EDGES; 
            else return graphlab::NO_EDGES;
        } // end of scatter_edges

        void scatter(icontext_type& context, const vertex_type& vertex,
                edge_type& edge) const {
#ifdef DEBUG_STEP_TIMER
            clock_t scatter_timer = clock();
#endif
            const vertex_type other = get_other_vertex(edge, vertex);
            hop_msg_type msg;
#ifdef TIE_FULL
            size_t pos = 0;
#endif //TIE_FULL
            for (std::vector<gsInstance>::const_iterator 
                    iter = inst_set.begin();
                    iter != inst_set.end();
                    ++iter) {
#ifdef TIE_FULL
                if (next_hop_set[pos].find(other.id()) != 
                        next_hop_set[pos].end()) {
                    gsInstance inst(*iter);
                    if (main_search_vid[pos] == other.id())
                        inst.state = Main;
                    else
                        inst.state = Started;
                    inst.path.push_back(other.id());
                    msg.inst_set.push_back(inst);
                }
                pos ++;
#else
                if (other.id() == iter->path.back()) 
                    msg.inst_set.push_back(*iter);
#endif //TIE_FULL
            }
            if (!msg.inst_set.empty())
                context.signal(other, msg);
#ifdef DEBUG_STEP_TIMER
            scatter_timer = clock() - scatter_timer;
            step_mtx.lock();
            agg_step_timer[2] += scatter_timer;
            step_mtx.unlock();
#endif
        } // end of scatter

        void save(graphlab::oarchive& oarc) const {
#ifdef TIE_FULL
            oarc << inst_set << next_hop_set << main_search_vid;
#else
            oarc << inst_set;
#endif //TIE_FULL
        }

        void load(graphlab::iarchive& iarc) {
#ifdef TIE_FULL
            iarc >> inst_set >> next_hop_set >> main_search_vid;
#else
            iarc >> inst_set;
#endif //TIE_FULL
        }

    };

#endif //if
