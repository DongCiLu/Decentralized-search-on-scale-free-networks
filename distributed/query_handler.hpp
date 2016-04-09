#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

#include "common.hpp"
#include "build_tree.hpp"
#include "dec_search.hpp"

#ifdef CALC_REAL
#include "real_dist.hpp"
#endif

class query_handler{
    public:
        query_handler(size_t n_tree, 
                size_t n_query,
                graphlab::distributed_control *dc,
                graphlab::command_line_options *clopts,
                graph_type *graph,
                std::string input_file,
                std::string exec_type,
                std::string saveprefix,
                std::string graph_dir,
                size_t n_query_batch,
                size_t n_tree_built = 0,
                long sum_real_dist = 0,
                long sum_tie_cnt = 0,
                double sum_appr_err = 0,
                double sum_obv_err = 0, 
                double bfs_runtime = 0,
                double gs_runtime = 0,
#ifdef CALC_REAL
                double real_bfs_runtime = 0,
#endif
                size_t err_cnt = 0) :
            n_tree(n_tree), n_query(n_query), 
            n_tree_built(n_tree_built), n_query_batch(n_query_batch),
            dc(dc), clopts(clopts), graph(graph), 
            input_file(input_file), exec_type(exec_type),
            saveprefix(saveprefix), graph_dir(graph_dir),
            sum_real_dist(sum_real_dist), sum_tie_cnt(sum_tie_cnt),
            sum_appr_err(sum_appr_err), sum_obv_err(sum_obv_err),
            bfs_runtime(bfs_runtime), gs_runtime(gs_runtime),
#ifdef CALC_REAL
            real_bfs_runtime(real_bfs_runtime),
#endif
            err_cnt(err_cnt) {
            // lower bound on number of experiments 
            this->n_query = 
                std::max(size_t(this->dc->numprocs()), this->n_query);
            this->n_query_batch = 
                std::min(this->n_query, this->n_query_batch);
            procid = this->dc->procid();
            in.open(this->input_file.c_str());
        }

        ~query_handler() {
            in.close();
        }

        void ds_query_batch() {
            dc->cout() << "\n1 Start building label system..." << std::endl;
            build_label(n_tree);

            for (size_t i = 0; i < n_query; i += n_query_batch) {
                dc->cout() << "\n2 Perform ds for queries..." << std::endl;
                dc->cout() << "\t2.0 Reading test cases..." << std::endl;
                results.resize(dc->numprocs());
                inst_set.resize(dc->numprocs());
                size_t n_query_to_perform = 
                    std::min(n_query_batch, n_query - i * n_query_batch);
                read_tcs(n_query_to_perform);

                dc->cout() << "\t2.1 Gather dest codes..." << std::endl;
                dst_set.resize(dc->numprocs());
                gather_dst();

                dc->cout() << "\t2.2 Create gs instances..." << std::endl;
                create_instances();

                dc->cout() << "\t2.3 Start greedy search... " << std::endl;
                start_ds_search();

                dc->cout() << "\t2.4 Gathering results..." << std::endl;
                ds_aggregate_results();

                dc->cout() << "\t2.5 Clean containers..." << std::endl;
                post_processing();
            }

            dc->cout() << "\n3 Writing results to file..." << std::endl;
            ds_write_results();

            dc->cout() << "\nDone." << std::endl;
        }

#ifdef CALC_REAL
        void real_query_serial() {
            dc->cout() << "\n1 Perform bfs for real dist..." << std::endl;
            dc->cout() << "\t1.0 Reading test cases..." << std::endl;
            results.resize(dc->numprocs());
            inst_set.resize(dc->numprocs());
            read_tcs(n_query); // dont use batch mode for real dist query

            dc->cout() << "\t1.2 Start distributed BFS..." 
                << std::endl;
            real_results.resize(dc->numprocs());
            start_bfs_search();

            dc->cout() << "\n2 Writing results to file..." << std::endl;
            real_write_results();

            dc->cout() << "\nDone." << std::endl;
        }
#endif

    private:
        void build_label(size_t n_tree_to_build) {
            graphlab::omni_engine<build_code_sys> bfs_engine(
                    *dc, *graph, "synchronous", *clopts); // asyn won't work 
            bfs_engine.elapsed_seconds(); // clear timer
            for(size_t t = n_tree_built; 
                    t < n_tree_built + n_tree_to_build; 
                    t++) {
                select_root_reducer r = 
                    graph->map_reduce_vertices<select_root_reducer>(
                            calc_root_rank);
                graphlab::vertex_id_type root_id = r.vid;
                bfs_engine.signal(root_id, min_distance_type(t, 0));
                bfs_engine.start();
                dc->cout() << "root: " << root_id << std::endl;
            }
            bfs_runtime += bfs_engine.elapsed_seconds();
            dc->cout() << "\tFinished building code system in " << 
                bfs_runtime << " seconds." << std::endl;
            n_tree_built += n_tree_to_build;
        }

        void read_tcs(size_t n_query_to_perform) {
            if (procid == masterid) {
                for (size_t i = 0; i < n_query_to_perform; i++) {
                    gsInstance inst;
#ifndef CALC_REAL
                    in >> inst.src_id >> inst.dst_id >> inst.real_dist;
#else
                    in >> inst.src_id >> inst.dst_id;
#endif
                    inst_set[procid].push_back(inst);
                }
            } 

            dc->barrier();
            dc->all_gather(inst_set);
        }

        void gather_dst() {
            std::vector<gsInstance> temp_set;
            temp_set.swap(inst_set[masterid]);
            for (size_t i = 0; i < temp_set.size(); ++i) {
                if (procid == graphlab::graph_hash::hash_vertex(
                            temp_set[i].dst_id)%dc->numprocs()) {
                    gsInstance inst(temp_set[i]);
                    inst.dst_code = graph->vertex(inst.dst_id).data().code;
                    dst_set[procid].push_back(inst);
                }
            }
            dc->barrier();
            dc->all_gather(dst_set);
        }

        void create_instances() {
            for (size_t i = 0; i < dst_set.size(); ++i) {
                for (size_t j = 0; j < dst_set[i].size(); ++j) {
                    if (procid == graphlab::graph_hash::hash_vertex(
                                dst_set[i][j].src_id)%dc->numprocs()) {
                        gsInstance inst(dst_set[i][j]);
                        inst.src_code = 
                            graph->vertex(inst.src_id).data().code;
                        inst.path.push_back(inst.src_id);
                        inst.min_dist = 
                            get_code_dist(inst.src_code, inst.dst_code);
                        // unique id, even for orig, odd for reverse
                        inst.id = (inst_set[procid].size() * 
                                dc->numprocs() + procid) * 2;
#ifdef TIE_FULL
                        inst.state = Main;
#else
                        inst.state = Started;
#endif
                        inst_set[procid].push_back(inst);
                    }
                }
            }

            dc->barrier();
            dc->all_gather(inst_set);
        }

        void start_ds_search() {
            std::vector<graphlab::vertex_id_type> src_set;
            std::vector<hop_msg_type> msg_set;
            for (size_t i = 0; i < inst_set.size(); ++i) {
                for (size_t j = 0; j < inst_set[i].size(); ++j) {
                    gsInstance inst = inst_set[i][j];
                    // forward search
                    src_set.push_back(inst.src_id);
                    msg_set.push_back(hop_msg_type(inst));
#ifdef BIDIRECTIONAL_SEARCH
                    gsInstance rev_inst(inst);
                    rev_inst.id = inst.id + 1;
                    rev_inst.src_id = inst.dst_id;
                    rev_inst.src_code = inst.dst_code;
                    rev_inst.dst_id = inst.src_id;
                    rev_inst.dst_code = inst.src_code;
                    rev_inst.path.clear();
                    rev_inst.path.push_back(rev_inst.src_id);
                    src_set.push_back(rev_inst.src_id);
                    msg_set.push_back(hop_msg_type(rev_inst));
#endif
                }
            }
            graphlab::omni_engine<dec_search> gs_engine(
                    *dc, *graph, exec_type, *clopts);

            gs_engine.signal_pairs(src_set, msg_set);
            dc->barrier();

            gs_engine.elapsed_seconds(); // clear timer
            gs_engine.start();
            gs_runtime += gs_engine.elapsed_seconds();
            dc->cout() << "\tFinished performing greedy search in " << 
                gs_runtime << " seconds." << std::endl;
            dc->gather(results, masterid);
        }

        void ds_aggregate_results() {
            if (procid == masterid) {
                std::map<size_t, gsInstance> comb_res;
                for (size_t i = 0; i < results.size(); i++) {
                    dc->cout() << "\t" << results[i].size() << 
                        " records on machine " << i << std::endl;
#ifdef TIE_FULL
                    for (std::map<size_t, gsInstance>::iterator 
                            iter = results[i].begin();
                            iter != results[i].end(); ++iter) {
                        // calc number of same length path
                        if (comb_res.find(iter->first) != comb_res.end() &&
                                comb_res[iter->first].path.size() ==
                                iter->second.path.size())
                            comb_res[iter->first].tie_cnt += 
                                iter->second.tie_cnt;
                        // discard longer path
                        if (comb_res.find(iter->first) == comb_res.end() ||
                                comb_res[iter->first].path.size() > 
                                iter->second.path.size())
                            comb_res[iter->first] = iter->second;
                    }
#else
                    comb_res.insert(results[i].begin(), results[i].end());
#endif
                }
                dc->cout() << "\t" << comb_res.size() 
                    << " records in total " << std::endl;

                for (std::map<size_t, gsInstance>::iterator 
                        iter = comb_res.begin();
                        iter != comb_res.end(); ++ iter) {
                    if (iter->second.path.empty() || 
                            iter->second.path.front() != 
                            iter->second.src_id ||
                            iter->second.path.back() != 
                            iter->second.dst_id) {
                        dc->cout() << "FATAL: wrong path!" << std::endl;
                        err_cnt ++;
                        continue;
                    }
#ifdef BIDIRECTIONAL_SEARCH
                    std::map<size_t, gsInstance>::iterator temp = iter++;
                    if (temp->second.path.size() == iter->second.path.size())
                        iter->second.tie_cnt += temp->second.tie_cnt;
                    if (temp->second.path.size() < iter->second.path.size()) {
                        std::swap(temp->second.path, iter->second.path);
                        iter->second.tie_cnt = temp->second.tie_cnt;
                    }

#endif
                    sum_real_dist += iter->second.real_dist;
                    if (iter->second.real_dist == 0) {
                        sum_appr_err += 0;
                        sum_obv_err += 0;
                    }
                    else {
                        sum_appr_err += double(iter->second.path.size() - 1) /
                            iter->second.real_dist - 1;
                        sum_obv_err += double(get_code_dist(
                                    iter->second.src_code, 
                                    iter->second.dst_code)) /
                            iter->second.real_dist - 1;
                    }
                    sum_tie_cnt += iter->second.tie_cnt;
                }
            }
        }

        void ds_write_results() {
            calc_overhead_reducer r = 
                graph->map_reduce_vertices<calc_overhead_reducer>(
                        calc_overhead);
            dc->barrier();
            if (procid == masterid) {
                // calculate the average visited vertices
                double avg_appr_err = 0, avg_real_dist = 0, 
                       avg_obv_err = 0, avg_tie_cnt = 0;

                avg_real_dist = double(sum_real_dist) / (n_query - err_cnt);
                avg_appr_err = double(sum_appr_err) / (n_query - err_cnt);
                avg_obv_err = double(sum_obv_err) / (n_query - err_cnt);
                avg_tie_cnt = double(sum_tie_cnt) / (n_query - err_cnt);

                std::string ofilename = saveprefix + ".txt";
                std::ofstream out(ofilename.c_str(), std::ios::out);
                out << "------- " << graph_dir << " -------" << std::endl;
                out << "BFS coding time is " << 
                    bfs_runtime << " seconds." << std::endl;
                out << "GS total running time (powergraph timer) is " << 
                    gs_runtime << " seconds." << std::endl;
                out << "Overheads(code, search, [reduced]) are " << 
                    r.code_overhead << " ";
                out << r.search_overhead / (n_query - err_cnt) << " ";
#ifdef SELECTIVE_LCA
                out << r.reduced_overhead / (n_query - err_cnt) << " ";
#endif
                out << std::endl;
                out << "Avg dist and est errors are (real, appr, obv): " << 
                    avg_real_dist << ", " << 
                    avg_appr_err << ", " <<
                    avg_obv_err << std::endl;
                out << "Average tie count is: " << avg_tie_cnt << std::endl;
                out << std::endl;
                out.close();
            }
        }

        void post_processing() {
            for (size_t i = 0; i < dc->numprocs(); i++) {
                results[i].clear();
                dst_set[i].clear();
                inst_set[i].clear();
            }
            results.clear();
            dst_set.clear();
            inst_set.clear();
        }

#ifdef CALC_REAL
        void start_bfs_search() {
            graphlab::omni_engine<real_sssp> real_sssp_engine(
                    *dc, *graph, "synchronous", *clopts); 
            std::map<graphlab::vertex_id_type, bfs_ds_type> sorted_inst;
            for (size_t i = 0; i < inst_set[masterid].size(); ++i) {
                inst_set[masterid][i].id = i;
                gsInstance inst(inst_set[masterid][i]);
                if (sorted_inst.find(inst.src_id) == sorted_inst.end())
                    sorted_inst[inst.src_id] = std::multimap<
                        graphlab::vertex_id_type, size_t>();
                sorted_inst[inst.src_id].insert(
                        std::make_pair(inst.dst_id, inst.id));
            }
            dc->barrier();
            size_t bfs_cnt = 0;
            for (typename std::map<graphlab::vertex_id_type, 
                    bfs_ds_type>::iterator 
                    iter = sorted_inst.begin();
                    iter != sorted_inst.end();
                    ++iter) {
                bfs_dst_set = iter->second;
                real_sssp_engine.signal(iter->first, 
                        real_min_distance_type(0));
                real_sssp_engine.start();
                dc->barrier();
                real_bfs_runtime += real_sssp_engine.elapsed_seconds();
                if (bfs_cnt++ % 10 == 9) {
                    dc->cout() << "\tFinished performing "<< bfs_cnt << 
                        " BFS in " << real_bfs_runtime << 
                        " seconds." << std::endl;
                }
            }
            dc->gather(real_results, masterid);
        }
#endif

#ifdef CALC_REAL
        void real_write_results(){
            if (procid == masterid) {
                std::map<size_t, gsInstance> comb_res;
                for (size_t i = 0; i < inst_set[masterid].size(); i++) {
                    comb_res.insert(std::make_pair(inst_set[masterid][i].id,
                                inst_set[masterid][i]));
                }
                for (size_t i = 0; i < real_results.size(); i++) {
                    dc->cout() << "\t" << real_results[i].size() << 
                        " real records on machine " << i << std::endl;
                    for (std::map<size_t, distance_type>::iterator 
                            iter = real_results[i].begin();
                            iter != real_results[i].end(); ++iter) {
                        comb_res[iter->first].real_dist = iter->second;
                    }
                }
                std::string ofilename = saveprefix + ".txt";
                std::ofstream out(ofilename.c_str(), std::ios::out);
                for (std::map<size_t, gsInstance>::iterator 
                        iter = comb_res.begin();
                        iter != comb_res.end(); ++iter) {
                    out << iter->second.src_id << " "
                        << iter->second.dst_id << " "
                        << iter->second.real_dist << std::endl;
                }
                out.close();
            }
        }
#endif

        size_t n_tree;
        size_t n_query;
        size_t n_tree_built;
        size_t n_query_batch;
        graphlab::distributed_control *dc;
        graphlab::command_line_options *clopts;
        graph_type *graph;
        std::string input_file;
        std::string exec_type;
        std::string saveprefix;
        std::string graph_dir;

        std::ifstream in;
        std::vector < std::vector<gsInstance> > dst_set;
        std::vector < std::vector<gsInstance> > inst_set;

        long sum_real_dist;
        long sum_tie_cnt;
        double sum_appr_err;
        double sum_obv_err; 
        double bfs_runtime;
        double gs_runtime;
#ifdef CALC_REAL
        double real_bfs_runtime;
#endif
        size_t err_cnt;

        const static unsigned int masterid = 0;
};

#endif
    
