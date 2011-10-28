require 'test/unit'
require 'pp'
# require_relative 'ruby/envutil'

class TestMemSysBase < Test::Unit::TestCase
  MemSys = GC::MemSys unless defined? MemSys
  DEFAULT_NAME = 'core'.freeze unless defined? DEFAULT_NAME

  attr_reader :argv0
  attr_reader :mem_sys, :mem_sys_name, :mem_sys_opts

  def initialize(*)
    super
    @tempfiles = [ ]
    @argv0 = $ruby || MemSys.argv0 # HACK
    @mem_sys = (ENV['RUBY_MEM_SYS'] || DEFAULT_NAME).dup.freeze
    @mem_sys_name, @mem_sys_opts = @mem_sys.split(':', 2)
    @mem_sys_opts ||= ''.freeze

    @mem_sys_name.freeze
    @mem_sys_opts.freeze
    $stderr.puts "Running under RUBY_MEM_SYS=#{mem_sys.inspect} MemSys.name=#{MemSys.name.inspect} (supports #{MemSys.supported})" if $DEBUG
    # $stderr.puts self.inspect
  end

  def tempfile(*args, &block)
    require 'tempfile'
    t = Tempfile.new(*args, &block)
    @tempfiles << t unless block
    t
  end

  def teardown
    @tempfiles.each { | f | f.close! }
  end

  def with_ENV env = { }
    save_ENV = { }
    ENV.each do | k, v |
      save_ENV[k.dup] = v.dup
    end
    ENV.update(env)

    yield

  ensure
    current_ENV = { }
    ENV.each do | k, v |
      current_ENV[k.dup] = v.dup
    end
    current_ENV.each do | k, v |
      if v = save_ENV[k]
        ENV[k] = v
      else
        ENV.delete(k)
      end
    end
  end

  def run_with_mem_sys! name, *args
    with_ENV 'RUBY_MEM_SYS' => name do
      args.unshift(argv0)
      $stderr.puts "\nrunning #{args.inspect} with RUBY_MEM_SYS=#{name.inspect}" if $DEBUG
      system(*args) or raise "#{args.inspect} failed"
    end
  end

  # Generic stats collector.
  # FIXME: move to separate file. -- kurt 2011/10/21
  class Stats
    def timer key
      slot = ((@timer ||= { })[key] ||= { })
      t1 = nil
      ::GC.disable
      t0 = Time.now
      yield
    rescue ::Exception => exc
      t1 = Time.now
      e = slot[:n_errors] ||= Hash.new(0)
      e[exc.class] += 1
      raise
    ensure
      t1 ||= Time.now
      ::GC.enable
      (slot[:t] ||= [ ]) << (t1.to_f - t0.to_f)
    end

    def keys
      @totals.keys
    end

    def values
      @totals.values
    end

    def stats k
      @stats ||= { }
      unless h = @stats[k]
        h = calc_stats(k)
      end
      h
    end

    # Calculate basic stats on an Array of Numeric.
    def calc_stats t
      case t
      when Symbol
        h = @stats[t] ||= { :key => t }
        t = @totals[t]
      else
        h = { }
      end
      t ||= [ ]
      t.sort!
      sum = t.inject(0){|v, e| v + e}
      avg = t.empty? ? nil : sum.to_f / t.size
      sdv = t.empty? ? nil : Math.sqrt(t.inject(0) { | s, x | s += (x - avg) ** 2 } / t.size)
      h[:sum] = sum
      h[:min] = t.first
      h[:avg] = avg
      h[:med] = t[t.size / 2]
      h[:max] = t[-1]
      h[:sdv] = sdv
      h[:n] = t.size
      h
    end

    def stats_ordered k
      h = stats(k)
      r = [ ]
      r << [ :"sum=", h[:sum] ]
      r << [ :"min=", h[:min] ]
      r << [ :"avg=", h[:avg] ]
      r << [ :"med=", h[:med] ]
      r << [ :"max=", h[:max] ]
      r = r.sort_by{|x| r[1] || 0}
      r << [ :"sdv=", h[:sdv] ]
      r.unshift([ :"n=", h[:n] ])
      r
    end

    def display_timers!
      slots = @timer ||= { }
      slots.keys.sort_by{|k| k.to_s}.each do | k |
        slot = slots[k]
        t = slot[:t] ||= [ ]
        r = calc_ordered(t)
        r << [ :n_errors, slot[:n_errors] ]
        pp [ :"#{k}_time", r ]
      end
      slots.clear
      self
    end

    def add_total! k, v
      ((@totals ||= { })[k] ||= [ ]) << v
      self
    end

    def display_totals!
      if slots = @totals
        slots.keys.sort_by{|k| k.to_s}.each do | k |
          r = stats_ordered(k)
          pp [ k, r ]
        end
        slots.clear
      end
      self
    end

    # Tracks stats per including Class or Module.
    module ModuleStats
      def stats
        @@stats ||= AmortizationSchedule::Stats.new
      end

      def timer n, &blk
        stats.timer(n, &blk)
      end

      def add_total! n, x
        stats.add_total!(n, x)
      end
    end
  end # class

end

