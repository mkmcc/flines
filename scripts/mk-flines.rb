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
#
require 'fileutils'
require 'tempfile'

# isolate the call to system() for debugging.
#
def issue_cmd(cmd)
  system cmd
end


# test whether a command exists on the system path
#
def command?(command)
  system("which #{ command} > /dev/null 2>&1")
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
  cmd_file = Tempfile.new(['mk-flines', '.sh'], '.')
  cmd_list.each do |cmd|
    cmd_file.write(cmd)
    cmd_file.write("\n")
  end
  cmd_file.flush

  parallel_cmd = "parallel --verbose --delay 2 < #{cmd_file.path}"

  IO.popen(parallel_cmd) do |io|
    io.each_line {|line| puts line}
  end

  cmd_file.close
end


# strip 4-digit numbers out of a string
# - eg, "cloud.0499.seed.lis" -> "0499"
#
def strip_digits(str)
  str.gsub(/.*\.([0-9]{4})\..*/, '\1')
end


# find the base name of a vtk file
# - 'cloud.0000.vtk' -> 'cloud'
# - 'id99/lev4/cloud.0449.seed.lis' -> 'cloud'
#
def get_base(str)
  # remove, eg, .dddd.vtk
  front = str.gsub(/(.*)\.([0-9]{4})\..*/, '\1')
  # remove, eg, id15/lev1/
  front.sub(/.*\//, '')
end



################################################################################

# <program>

unless File.executable?('./flines')
  puts "###error: flines not found"
  exit 1
end

unless File.readable?('./input.fline')
  puts "###error: input.fline not found"
  exit 1
end

# get the basename
base = get_base(Dir.glob('*.vtk').first)


# get pairs of vtk and seed files
vtk_files  = Dir.glob('*.vtk').map{|f| strip_digits(f)}
seed_files = Dir.glob('*.lis').map{|f| strip_digits(f)}
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
end

puts "running #{cmds.length} files..."

if command?('parallel')
  pipe_to_parallel(cmds)
else
  cmds.each do |cmd|
    issue_cmd(cmd)
  end
end

# </program>


exit 0
