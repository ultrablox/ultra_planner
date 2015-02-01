
main_file = File.new("all_data.txt", "r")


i = 1
while (inst = main_file.gets)
    file = File.open("korf_" + i.to_s.rjust(3, "0") + ".txt", "w")
    file.write("4 4\n")
    file.write(inst)
    file.close
    i += 1
end
