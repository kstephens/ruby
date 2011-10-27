require 'test/unit'
require 'tempfile'
require 'pp'
# require_relative 'ruby/envutil'

class TestMemSys < Test::Unit::TestCase
  MemSys = GC::MemSys
  attr_reader :argv0
  attr_reader :mem_sys, :mem_sys_name, :mem_sys_opts
  DEFAULT_NAME = 'core'.freeze

  def initialize(*)
    super
    @argv0 = MemSys.argv0 # HACK
    @mem_sys = (ENV['RUBY_MEM_SYS'] || DEFAULT_NAME).dup.freeze
    @mem_sys_name, @mem_sys_opts = @mem_sys.split(':', 2)
    @mem_sys_opts ||= ''.freeze

    @mem_sys_name.freeze
    @mem_sys_opts.freeze
    $stderr.puts "Running under RUBY_MEM_SYS=#{mem_sys.inspect} MemSys.name=#{MemSys.name.inspect} (supports #{MemSys.supported})"
    # $stderr.puts self.inspect
  end

  def tempfile(*args, &block)
    t = Tempfile.new(*args, &block)
    @tempfile = (t unless block)
  end

  def teardown
    @tempfile.close! if @tempfile
  end


  def test_mem_sys_name
    assert_equal(mem_sys_name, MemSys.name)
  end

  def test_mem_sys_opts
    assert_equal(mem_sys_opts, MemSys.opts)
  end

  def test_mem_sys_supported
    assert_equal([ DEFAULT_NAME, 'malloc' ], MemSys.supported)
  end

  def test_mem_sys_malloc
    return unless MemSys.name == DEFAULT_NAME
    name = 'malloc'
    run_with_mem_sys! name, "-e", "raise unless GC::MemSys.name == '#{name}'"
    run_with_mem_sys! name, __FILE__
    run_with_mem_sys! name, File.expand_path("../../tool/rubytest.rb", __FILE__)
    # run_with_mem_sys! 'malloc', 'make', 'test-all'
  end

  def run_with_mem_sys! name, *args
    $stderr.puts "MemSys.name = #{MemSys.name.inspect} switching to #{name.inspect}"
    unless MemSys.name == name.split(":", 2)
      ENV['RUBY_MEM_SYS'] = name
      args.unshift(argv0)
      $stderr.puts "\nrunning #{args.inspect}"
      system(*args) or raise "#{args.inspect} failed"
    end
  end

  def test_mem_sys_event_log
    file = tempfile(['mem_sys_event_log', '.txt'])
    ENV['RUBY_MEM_SYS_EVENT_LOG'] = file.path
    args = [ argv0, '-e', 'puts (0..10).to_a.inspect' ]
    pid = Kernel.fork do
      exec(*args)
      raise "#{args.inspect} failed"
    end
    $stderr.puts "#{args * ' '} pid => #{pid}"
    Process.waitpid(pid) or raise "failed?"

    o_a = [ ]; o_f = [ ]
    p_a = [ ]; p_f = [ ]
    e = [ ]

    contents = File.open(file.to_s) { |fh| fh.readlines }
    lineno = 0
    contents.each do | l |
      lineno += 1
      l.chomp!

      case l
      when /^#{pid} (\d+) (\d+) o\{ (0x[0-9a-f]+) (\d+)$/i
        o_a << l
      when /^#{pid} (\d+) (\d+) o\} (0x[0-9a-f]+) (\d+)$/i
        o_f << l
      when /^#{pid} (\d+) (\d+) p\{ (0x[0-9a-f]+) (\d+)$/i
        p_a << l
      when /^#{pid} (\d+) (\d+) p\} (0x[0-9a-f]+) (\d+)$/i
        p_f << l
      when /^#{pid} (\d+) (\d+) EXIT$/
        e << l
      else
        raise "Invalid event at event log line #{lineno}: #{l.inspect}"
      end
    end # each
    pp [ o_a.size, o_f.size, p_a.size, p_f.size, e.size ]
    assert_equal(1, e.size)
    assert(o_a.size >= o_f.size)
    assert(p_a.size >= p_f.size)
    # assert_equal(o_a.size, o_f.size, "object frees == object alloc") # core gc does not abide
    # assert_equal(p_a.size, p_f.size, "page frees == page alloc")
  end
end

