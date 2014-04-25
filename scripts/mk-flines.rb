require 'fileutils'
require 'tempfile'

# isolate the call to system() for debugging.
#
def issue_cmd(cmd)
  system cmd
end

vtk_files  = Dir.glob('*.vtk').map{|f| f.gsub(/.*\.([0-9]{4})\.vtk/, '\1')}
seed_files = Dir.glob('*.lis').map{|f| f.gsub(/.*\.([0-9]{4})\.seed.lis/, '\1')}
base       = Dir.glob('*.0000.vtk').first.gsub('.0000.vtk', '').gsub('id0/', '')

# intersect vtk_files and seed_files
nums = vtk_files & seed_files

cmds = []

nums.each do |num|
  vtkfile  = "#{base}.#{num}.vtk"
  seedfile = "#{base}.#{num}.seed.lis"

  outfile  = "#{base}.#{num}.vtk.flines"

  unless FileUtils.uptodate?(outfile, [vtkfile, seedfile])
    str = "./flines -i input.fline"
    str += " files/vtk_file=#{vtkfile}"
    str += " files/out_file=#{outfile}"
    str += " initial_condition/seed_file=#{seedfile}"

    cmds << str
  end
end

if cmds.empty?
  puts "all up to date"
  exit 0
end

cmdfile = Tempfile.new(['mk-flines', '.sh'], '.')
cmds.each do |cmd|
  cmdfile.write(cmd)
  cmdfile.write("\n")
end
cmdfile.flush

parallel_cmd = "parallel --verbose --delay 2 < #{cmdfile.path}"

IO.popen(parallel_cmd) do |f|
  f.each_line {|line| puts line}
end

cmdfile.close
