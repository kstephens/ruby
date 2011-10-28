require 'test/unit'
require 'pp'
require_relative 'test_mem_sys_base'

class TestMemSys < TestMemSysBase
  def initialize(*)
    super
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
    name = 'malloc'
    return if MemSys.name == name
    run_with_mem_sys! name, "-e", "raise unless GC::MemSys.name == '#{name}'"
    run_with_mem_sys! name, __FILE__
    run_with_mem_sys! name, File.expand_path("../../tool/rubytest.rb", __FILE__)
    # run_with_mem_sys! 'malloc', 'make', 'test-all'
  end

end

