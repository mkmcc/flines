# mk-flines.rb: call `flines' where it is needed
#
# NOTES:
#
# 1. This script is meant to be used for making movies.
#
# 2. I'm assuming you have a directory with the merged vtk files and
#    with the `seed' files listing starting points for the field
#    lines.
#
# 3. The vtk files should be named base.dddd.vtk, and the seed files
#    should be named base.dddd.seed.lis
#
# 4. This script finds pairs of vtk and seed files which do not have a
#    streamline file.  For those, it runs the `flines' program and
#    saves the output in base.dddd.flines
#
# 5. slightly hackish: I don't call the `flines' program directly;
#    instead, I write a shell script with the calls and pipe that into
#    gnu parallel.
#
#
# TODO:
#
# 1. Detect whether parallel exists; if not run in serial.
#
# 2. Make the call to gnu parallel less ugly... maybe using ruby
#    parallel? https://github.com/grosser/parallel
#
require 'fileutils'
require 'tempfile'

# isolate the call to system() for debugging.
#
def issue_cmd(cmd)
  system cmd
end


# command to run flines once
#
def flines_cmd(vtkfile, seedfile, outfile)
  str = "./flines -i input.fline"
  str += " files/vtk_file=#{vtkfile}"
  str += " files/out_file=#{outfile}"
  str += " initial_condition/seed_file=#{seedfile}"

  return str
end


# send a list of commands to gnu parallel.
# - this is a bit hackish... writes a shell script with the list of
#   commands, then issues the command 'parallel < tmp.sh'
#
def pipe_to_parallel(cmd_list)
  # save the commands to a temporary file so we can feed it into gnu
  # parallel
  #
  cmdfile = Tempfile.new(['mk-flines', '.sh'], '.')
  cmd_list.each do |cmd|
    cmdfile.write(cmd)
    cmdfile.write("\n")
  end
  cmdfile.flush

  parallel_cmd = "parallel --verbose --delay 2 < #{cmdfile.path}"

  IO.popen(parallel_cmd) do |f|
    f.each_line {|line| puts line}
  end

  cmdfile.close
end


# get the basename
# FIXME: this requires 0000.vtk file to exist
base       = Dir.glob('*.0000.vtk').first.gsub('.0000.vtk', '').gsub('id0/', '')


# get pairs of vtk and seed files
vtk_files  = Dir.glob('*.vtk').map{|f| f.gsub(/.*\.([0-9]{4})\.vtk/, '\1')}
seed_files = Dir.glob('*.lis').map{|f| f.gsub(/.*\.([0-9]{4})\.seed.lis/, '\1')}
nums = vtk_files & seed_files   # & means 'intersect'


# build a list of commands to update field line files
cmds = []
nums.each do |num|
  vtkfile  = "#{base}.#{num}.vtk"
  seedfile = "#{base}.#{num}.seed.lis"
  outfile  = "#{base}.#{num}.flines"

  unless FileUtils.uptodate?(outfile, [vtkfile, seedfile])
    cmds << flines_cmd(vtkfile, seedfile, outfile)
  end
end

if cmds.empty?
  puts "all up to date"
  exit 0
else
  puts "running #{cmds.length} files..."
  pipe_to_parallel(cmds)
end

exit 0
