
PLANNER_PATH = ARGV[0]
DOMAINS_DIR = ARGV[1]

puts "Starting experiment..."

if(!File.file?(PLANNER_PATH))
	puts "Planner executable not found!"
	exit 1
end

if(File.file?("results.csv"))
	File.delete("results.csv")
end

def run_experiment(batch_factor, start_number)

	file = File.open("results_" + (batch_factor/1000).to_s + "K.csv", "w")

	cur_domain = start_number

	while(true)
		domain_name = File.absolute_path(DOMAINS_DIR + "/p" + cur_domain.to_s.rjust(2, "0") + "-domain.pddl")
		problem_name = File.absolute_path(DOMAINS_DIR + "/p" + cur_domain.to_s.rjust(2, "0") + ".pddl")
		
		if((!File.file?(domain_name)) and (!File.file?(problem_name)))
			break;
		end
		
		puts "Launching planner with " + domain_name + "+" + problem_name
		
		call_cmd = PLANNER_PATH + ' --solver_type mt --optimal 1 --solver_max_ouput_size ' + batch_factor.to_s + ' "' + domain_name + '" "' + problem_name + '"'
		puts "Running: " + call_cmd
		
		system(call_cmd)
		
		stats_file = File.new("stats.txt", "r")
		stats = Hash.new
		while (line = stats_file.gets)
			parts = line.split(':') 
			stats[parts[0]] = parts[1]
		end
		stats_file.close
		File.delete("stats.txt")

		file.write(cur_domain.to_s + ';' + stats["search_time"].to_f.to_s + "\n") 
		file.flush
		
		cur_domain += 1
	end

	file.close
end

run_experiment(1000, 12)

