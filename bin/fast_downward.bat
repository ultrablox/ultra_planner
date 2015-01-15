python ..\..\LementPlanner\planners\fast-downward\translate\translate.py d_1.pddl p_1.pddl --force
..\..\LementPlanner\planners\fast-downward\preprocess.exe < output.sas
..\..\LementPlanner\planners\fast-downward\downward-1.exe --search Astar(blind) < output