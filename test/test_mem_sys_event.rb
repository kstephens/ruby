require 'test/unit'
require 'tempfile'
require 'pp'

class TestMemSysEvent < Test::Unit::TestCase
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

    e = Hash.new{ |h, k| h[k] = [ ] }
    s = Stats.new

    contents = File.open(file.path) { |fh| fh.readlines }
    lineno = 0
    contents.each do | l |
      lineno += 1
      l.chomp!

      case l
      when /^#{pid} (\d+) (\d+) (oa|of|pa|pf) (0x[0-9a-f]+) (\d+)$/i
        x = {
          :event_id => $1.to_i,
          :object_alloc_id => $2.to_i,
          :tag => $3.to_sym,
          :addr => $4,
          :size => $5.to_i,
          :line => l,
        }
        e[x[:tag]] << x
        s.add_total!(:"#{x[:tag]}_size", x[:size])
      when /^#{pid} (\d+) (\d+) EXIT$/
        e[:EXIT] << l
      else
        raise "Invalid event at event log line #{lineno}: #{l.inspect}"
      end
    end # each
    if $DEBUG
      pp [
      MemSys.name,
      :e_count, e.map { | k, v | [ k, v.size ] },
         ]
      s.display_totals!
    end

    assert(lineno > 100)
    assert(e[:oa].size >= e[:of].size)
    assert(e[:pa].size >= e[:pf].size)
    assert_equal(1, e[:EXIT].size)
    case MemSys.name
    when 'core'
      assert_equal(0.0, s.stats(:of_size)[:sdv])
    when 'malloc'
      assert_equal(0, e[:pa].size)
      assert_equal(0, e[:pf].size)
      assert_equal(0, t[:pa])
      assert_equal(0, t[:pf])
    end
    # assert_equal(e[:oa].size, e[:of].size, "object frees == object alloc") # core gc does not abide
    # assert_equal(e[:pa].size, e[:pf].size, "page frees == page alloc")
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

