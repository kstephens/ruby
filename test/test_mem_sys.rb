require 'test/unit'
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

end

