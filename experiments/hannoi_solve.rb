
require 'fileutils'
include FileUtils

SOLVER_PATH = ARGV[0]
DOMAINS_DIR = ARGV[1]

puts "Starting experiment..."

if(!File.file?(SOLVER_PATH))
	puts "Solver executable not found!"
	exit 1
end

def prepare_external_storage
	disks = ['D:', 'F:', 'G:']
	disks.each do |d|
		for fn in 0..3
			call_cmd = 'stxxl_tool.exe create_files 3221225472 ' + d + '\stxxl_' + fn.to_s + '.tmp'
			puts "Running: " + call_cmd
			system(call_cmd)
		end
	end
end

def solve(domain_name, algorithm_name)
	call_cmd = SOLVER_PATH + ' --algorithm ' + algorithm_name + ' --storage ext "' + domain_name + '"'
	puts "Running: " + call_cmd
	system(call_cmd)
	
	stats_file = File.new("stats.txt", "r")
	stats = Hash.new
	while (line = stats_file.gets)
		parts = line.split(':') 
		stats[parts[0]] = parts[1]
	end
	stats_file.close
	
	FileUtils.copy_file("stats.txt", domain_name + "_stats.txt")
	FileUtils.copy_file("monitoring_data.csv", domain_name + "_monitoring_data.csv")
	
	File.delete("stats.txt")
	
	return stats
end

algs = ["A*"] #, ,, , , "GBFS", "BA*"
columns = ["wall_time", "plan_length", "plan_cost", "node_count", "nodes_dump_size"]

if(File.file?("results.csv"))
	File.delete("results.csv")
end

#prepare_external_storage()

file = File.open("results.csv", "w")

#Print header
file.write(";") 
algs.each do |alg|
	file.write(alg)
	columns.each do |c|
		file.write(";")
	end
end
file.write("\n")

file.write("problem_name;") 
algs.each do |alg|
	columns.each do |c|
		file.write(c + ";")
	end
end
file.write("\n")

file.flush

#Solve and print data


items = []

Dir.foreach(DOMAINS_DIR) do |item|
	next if item == '.' or item == '..'
	items.push(item)
end


items.each do |item|
	file.write(item + ";")
	
	algs.each do |alg|
		stats = solve(File.absolute_path(DOMAINS_DIR + '/' + item), alg)
		
		columns.each do |c|
			file.write(stats[c].to_f.to_s + ";")
		end
		file.flush
	end
	
	file.write("\n")
end

file.close


