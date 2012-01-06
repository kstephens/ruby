=begin

Feature: Raise any object

= Proposal

The ability to raise any object that conforms to the protocol of Exception.

= Problem

* The Exception subclass hierarchy is well-established.
* CRuby does not allow any object that behaves as an Exception to be raised, it must be a subclass of Exception.
* 3rd-party code often rescues Exception; e.g. for error recovery, retry and/or logging.
* Users need the ability to raise objects that would not normally be rescued by *any* code;
  e.g.: hard timeouts or custom signal handlers in an application.

= Solution

* ruby/eval.c: Remove make_exception() assertion rb_obj_is_kind_of(mesg, rb_mRaiseable).
* Clean up Execption ivar names (@bt, @mesg) and method names (set_backtrace).

Sample Usage:

=end

require 'test/unit'

class TestRaiseable < Test::Unit::TestCase
  # Example: mixin implementation of Exception protocol methods:
  module Raiseable
    def exception *args # error.c: exc_exception()
      case args.size
      when 0
        return self
      when 1
        return self if self.object_id == args[0].object_id
      end
      clone.initialize *args
    end
    def initialize mesg = nil # error.c: exc_initialize()
      @mesg = mesg
      @bt = nil
      self
    end
    def == other # error.c: exc_equal()
      return true if self.object_id == other.object_id
      if self.class == other.class
        mesg = other.message
        backtrace = other.backtrace
      else
        mesg = other.instance_variable_get(:'@mesg')
        backtrace = other.backtrace
      end
      @mesg == mesg && self.backtrace == backtrace
    end
    def to_s # error.c: exc_to_s()
      @mesg || self.class.name
    end
    def message; to_s; end
    def inspect # error.c: exc_inspect()
      "#<#{self.class}: #{self}>"
    end
    def backtrace; @bt; end
    def set_backtrace v # error.c: exc_set_backtrace()
      v = [ v ] if String === v
      raise TypeError, "backtrace must be Array of String" unless Array === v || v.all?{|e| String === e}
      @bt = v
    end
    def self.included target
      super
      target.extend ModuleMethods
    end
    module ModuleMethods
      def exception *args; new *args; end
    end
  end

  def test_raiseable
    raiseable = Class.new do
      include Raiseable
    end
    begin
      raise raiseable, "this must be handled"
      assert(false)
    rescue Exception
      $stderr.puts "#{__FILE__}:#{__LINE__}: Unexpected Exception: #{$!.inspect}"
      assert(false)
    rescue Raiseable
      assert(true)
    end
  end

  class SomeLibraryFromSomebodyElse
    def do_it
      something_that_takes_too_long
    rescue ::Exception => exc
      $stderr.puts "\n#{__FILE__}:#{__LINE__}: Something failed: #{exc.inspect}"
      42
    end

    def something_that_takes_too_long
      sleep 10
    end
  end

  class MyTimeoutError
    include Raiseable
  end

  def test_MyTimeoutError
    raise MyTimeoutError
    assert(false)
  rescue ::Exception
    $stderr.puts "#{__FILE__}:#{__LINE__}: Unexpected Exception: #{$!.inspect}"
    assert(false)
  rescue MyTimeoutError
    assert(true)
  end

  def test_raiseable_timeout
    require 'timeout'
    begin
      Timeout.timeout(1, MyTimeoutError) do
        SomeLibraryFromSomebodyElse.new.do_it
      end
      assert(false)
    rescue MyTimeoutError
      assert(true)
    rescue Exception
      $stderr.puts "#{__FILE__}:#{__LINE__}: Unexpected Exception: #{$!.inspect}"
      assert(false)
    end
  end
end

