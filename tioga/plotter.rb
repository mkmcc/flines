require 'Tioga/FigureMaker'
require 'plot_styles.rb'
require 'Dobjects/Function'

require 'matrix'

class MyPlots

  include Math
  include Tioga
  include FigureConstants
  include MyPlotStyles

  def t
    @figure_maker
  end

  def initialize
    @figure_maker = FigureMaker.default

    t.save_dir = 'plots'

    t.def_figure('flines') do
      mnras_style
      enter_page
      flines
    end
  end

  def enter_page
    sans_serif_style
    mnras_style

    t.default_frame_left   = 0.12
    t.default_frame_right  = 0.98
    t.default_frame_top    = 0.96
    t.default_frame_bottom = 0.12

    t.default_page_width  = 72 * 3.5

    t.default_page_height = t.default_page_width * \
      (t.default_frame_right - t.default_frame_left) / \
      (t.default_frame_top - t.default_frame_bottom)

    t.default_enter_page_function
  end

  def read_fline(istart)
    retvar = []
    i=istart
    d = Dvector.read_row('flines.out', i)
    while (d.length == 3)
      retvar << d
      i += 1
      d = Dvector.read_row('flines.out', i)
    end

    [i+1, retvar]
  end

  def read_flines()
    flines = []
    i = 1
    inext, fline = read_fline(i)
    while (fline.length >= 1)
      flines << fline
      inext, fline = read_fline(inext)
    end

    flines
  end

  def project_fline(fline)
    alpha = -1.2 * 3.14159
    beta  = 0.6
    dist  = 3.0

    mat1 = Matrix[ [cos(-beta),   0,   -sin(-beta)],
                   [         0,   1,             0],
                   [sin(-beta),   0,    cos(-beta)] ]

    mat2 = Matrix[ [cos(-alpha),   -sin(-alpha),   0],
                   [sin(-alpha),    cos(-alpha),   0],
                   [          0,              0,   1] ]

    mat = mat1 * mat2

    proj = Matrix[[0, 1, 0], [0, 0, 1]]

    camera = Matrix.column_vector( [dist * cos(beta) * cos(alpha),
                                    dist * cos(beta) * sin(alpha),
                                    dist * sin(beta)])
    c = mat * camera


    fline.map do |vec|
      v = mat * Matrix.column_vector(vec.to_a)
      r = (v[0,0]-c[0,0]).abs

      v = (proj * v) / r
      v.transpose.to_a.first    # wtf???
    end
  end

  def project_flines(flines)
    flines.map do |fline|
      project_fline(fline)
    end
  end

  def flines
    flines = read_flines[0..50]

    puts flines.length
    puts flines[0].length

    flines = project_flines(flines)

    t.line_width = 0.0

    t.show_plot([-0.15, 0.15, 0.15, -0.15]) do
      flines.each do |fline|
        xvals = fline.map {|v| v[0]}
        yvals = fline.map {|v| v[1]}

        t.show_polyline(xvals, yvals)
      end
    end
  end

end

MyPlots.new

# Local Variables:
#   compile-command: "tioga plotter.rb -s"
# End:
