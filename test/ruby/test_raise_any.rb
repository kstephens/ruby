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
  # Sample mixin implementation of Exception protocol.
  module Raiseable
    attr_accessor :message, :backtrace
    def initialize m = nil # see error.c:exc_initialize().
      $stderr.puts "\n  #{self.class} Raiseable#initialize (#{m.inspect})"
      @message = m
      self
    end
    def to_s # see error.c:exc_to_s()
      @message || self.class.name
    end
    def exception *args # see error.c:exc_exception().
      $stderr.puts "\n  #{self.class} Raiseable exception #{args.inspect}"
      self.dup.send(:initialize, *args)
    end
    def self.included target
      $stderr.puts "\n  #{self}.included #{target}"
      super
      target.extend ModuleMethods
    end
    module ModuleMethods
      def exception *args
        $stderr.puts "\n  #{self} Raiseable::ModuleMethods exception #{args.inspect}"
        new *args
      end
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
      assert(false)
    rescue Raiseable
      assert(true)
    end
  end

  class SomeLibraryFromSomebodyElse
    def do_it
      something_that_takes_too_long
    rescue ::Exception => exc
      $stderr.puts "\nSomething failed: #{exc.inspect}"
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
    assert(false)
  rescue MyTimeoutError
    # $stderr.puts "TIMEOUT: #{$!.inspect}"
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
      # $stderr.puts "TIMEOUT: #{$!.inspect}"
      assert(true)
    end
  end
end

