#include <iostream>
#include <string>
#include <fstream>

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
#ifdef TIE_FULL
    size_t n_query_batch = 1000;
#else
    size_t n_query_batch = 50000;
#endif
    query_handler qh(n_tree, n_query,
            &dc, &clopts, &graph, 
            input_file, exec_type, 
            saveprefix, graph_dir,
            n_query_batch);

#ifndef CALC_REAL
    qh.ds_query_batch();
#else
    qh.real_query_serial();
#endif

    // Tear-down communication layer and quit ----------------------
    graphlab::mpi_tools::finalize();
    return EXIT_SUCCESS;
} // End of main

