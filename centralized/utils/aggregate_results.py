import os
import argparse

dirname = "../../results/expand/{}/"
graph_seq = ['bio', 'econ', 'hollywood', 'Livejournal', 'orkut', 'protein', 'skitter', 'wiki']

if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('treetype', type=str)
    arg_parser.add_argument('tietype', type=str)
    args = arg_parser.parse_args()
    outfilename = args.treetype + "_" + args.tietype + "_detail.txt"
    suffix = args.tietype + "_detail.txt"
    dirname = dirname.format(args.treetype)

    with open(outfilename, 'w') as outf:
        for subdir, dirs, files in os.walk(dirname):
            for f in files:
                fn = os.path.join(subdir, f)
                if suffix in f:
                    with open(fn) as inf:
                        est_save = 0
                        comp_save = 0
                        est_exact = 0
                        comp_exact = 0
                        for line in inf:
                            segs = line.split(' ')
                            sp = int(segs[2])
                            est = int(segs[3])
                            comp = int(segs[4])
                            obv = int(segs[5])

                            if obv > est:
                                est_save += 1
                            if obv > comp:
                                comp_save += 1
                            if est == sp:
                                est_exact += 1
                            if comp == sp:
                                comp_exact += 1

                        outf.write("{} {} {} {} {}\n".format(
                            f, est_save, comp_save, 
                            est_exact, comp_exact))
