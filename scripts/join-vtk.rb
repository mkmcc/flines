require 'fileutils'

# isolate the call to system() for debugging.
#
def issue_cmd(cmd)
  system cmd
end


issue_cmd 'mkdir -p merged'

dirs  = Dir.glob('id[^0]*').map{|f| f.sub(/^id/, '')}
files = Dir.glob('id0/*.vtk').map{|f| f.gsub(/.*\.([0-9]{4})\.vtk/, '\1')}
base  = Dir.glob('id0/*.0000.vtk').first.gsub('.0000.vtk', '').gsub('id0/', '')

files.each do |num|
  outfile = "merged/#{base}.#{num}.vtk"

  infiles = ["id0/#{base}.#{num}.vtk"]
  infiles = infiles + dirs.map{|d| "id#{d}/#{base}-id#{d}.#{num}.vtk"}

  unless FileUtils.uptodate?(outfile, infiles)
    str = infiles.join(" ")
    cmd = "./join_vtk.x -o #{outfile} " + str
    issue_cmd cmd
  end
end
