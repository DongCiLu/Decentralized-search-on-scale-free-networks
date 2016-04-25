#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <cstdlib>
#include <time.h>

#include "common.hpp"
#include "build_tree.hpp"
#include "dec_search.hpp"
#include "query_handler.hpp"

#ifdef CALC_REAL
#include "real_dist.hpp"
#endif

int main(int argc, char** argv) {
    // Initialize control plain using mpi
    graphlab::mpi_tools::init(argc, argv);
    graphlab::distributed_control dc;
    global_logger().set_log_level(LOG_FATAL);

    // Parse command line options -------------------------------------
    graphlab::command_line_options 
        clopts("Distributed Path Finding Algorithm.");
    std::string graph_dir;
    std::string format = "tsv";
    std::string exec_type = "synchronous";
    clopts.attach_option("graph", graph_dir,
            "The graph file.  If none is provided "
            "then a toy graph will be created");
    clopts.add_positional("graph");

    clopts.attach_option("engine", exec_type, 
            "The engine type synchronous or asynchronous");

    std::string saveprefix;
    clopts.attach_option("saveprefix", saveprefix,
            "If set, will save the resultant pagerank to a "
            "sequence of files with prefix saveprefix");
    
    size_t n_tree = 1;
    clopts.attach_option("num_tree", n_tree,
            "Number of trees.");

    int stepy = 0;
    clopts.attach_option("stepy", stepy,
            "Normal mode or stepy mode.");
   
    size_t n_query = 0;
    clopts.attach_option("num_query", n_query,
            "Preset number of experiments.");

    std::string input_file = "";
    clopts.attach_option("input_file", input_file,
            "Read from file or randomly generate (default).");

    if(!clopts.parse(argc, argv)) {
        dc.cout() << 
            "Error in parsing command line arguments." << std::endl;
        return EXIT_FAILURE;
    }

    // Build the graph ---------------------------------------------
    dc.cout() << "\n0 Loading graph..." << std::endl;
    graph_type graph(dc, clopts);
    dc.cout() << "\tLoading graph in format: "<< format << std::endl;
    graph.load_format(graph_dir, format);
    // must call finalize before querying the graph
    graph.finalize();
    dc.cout() << "\t#vertices:  " << graph.num_vertices() << 
        "\t#edges:     " << graph.num_edges() << std::endl;

    // start the handler here
/*
#ifdef TIE_FULL
#ifdef TIE_HEUR
    size_t n_query_batch = 10000;
#else
    size_t n_query_batch = 1000;
#endif
#else
    //size_t n_query_batch = n_query * 2;
    size_t n_query_batch = 50000;
#endif
#ifdef BIDIRECTIONAL_SEARCH
    n_query_batch /= 2;
#endif
*/
    size_t n_query_batch = n_query;

#ifndef CALC_REAL
    if (stepy == 0) {
        query_handler qh(n_tree, n_query, 
                &dc, &clopts, &graph, 
                input_file, exec_type, 
                saveprefix, graph_dir,
                n_query_batch);
        qh.ds_query_batch();
    }
    else {
        for (size_t n_tree_built = 0; 
                n_tree_built < n_tree; 
                n_tree_built ++) {
            std::ostringstream oss;
            oss << saveprefix << "_" << n_tree_built + 1 << "t";
            std::string saveprefix_mod = oss.str();
            query_handler qh(n_tree_built + 1, n_query, 
                    &dc, &clopts, &graph, 
                    input_file, exec_type, 
                    saveprefix_mod, graph_dir,
                    n_query_batch, n_tree_built);
            qh.ds_query_batch();
        }
    }
#else
    query_handler qh(n_tree, n_query, 
            &dc, &clopts, &graph, 
            input_file, exec_type, 
            saveprefix, graph_dir,
            n_query_batch);
    qh.real_query_serial();
#endif

    // Tear-down communication layer and quit ----------------------
    graphlab::mpi_tools::finalize();
    return EXIT_SUCCESS;
} // End of main

