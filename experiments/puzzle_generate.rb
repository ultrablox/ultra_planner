
SOLVER_PATH = ARGV[0]
DOMAINS_DIR = ARGV[1]

puts "Starting experiment..."

if(!File.file?(SOLVER_PATH))
	puts "Solver executable not found!"
	exit 1
end

def generate(width, height, permutation_count)
	problem_name = File.absolute_path(DOMAINS_DIR + "/puzzle" + width.to_s + "x" + height.to_s + "-" + permutation_count.to_s + ".txt")
	call_cmd = SOLVER_PATH + ' --command generate --width ' + width.to_s + ' --height ' + height.to_s + ' --permutations ' + permutation_count.to_s + ' "' + problem_name + '"'
	puts "Running: " + call_cmd
	system(call_cmd)
end

#testing_permutations = [5, 10, 20, 50, 100, 200]
testing_permutations = [400]
#testing_sizes = [[2,6], [3,4], [4,3], [6,2], [7,2], [4,4], [3,5], [4,5]] #[5,5]
testing_sizes = [[5,5], [6,6]]

testing_sizes.each do |info|
	testing_permutations.each do |p|
		generate(info[0], info[1], p)
	end
end

