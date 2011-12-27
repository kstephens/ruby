=begin

Feature: Throwable

This proposes the ability to throw any object that is a Throwable.

Problem:

* The Exception subclass hierarchy is well-established.
* CRuby does not allow any object that behaves as an Exception to be thrown, it must be a subclass of Exception.
* 3rd-party code often rescues Exception; e.g. for error recovery, retry and/or logging.
* Users need the ability to raise objects that would not normally be rescued by *any* code;
  e.g.: hard timeouts or custom signal handlers in an application.

Solution:

* A "Throwable" module implements all of the methods currently defined in Exception.
* Exception class includes Throwable module.
* ruby/eval.c: make_exception() asserts rb_obj_is_kind_of(mesg, rb_mThrowable), 
instead of rb_obj_is_kind_of(mesg, rb_cException).  
* Users should avoid "rescue Throwable" in usual circumstances.

Other Ideas not Implemented here:

* Remove the obj_is_kind_of(mesg, rb_mThrowable) restriction to allow pure duck-typing.
* Clean up the ivar names (@bt, @mesg) and method names (set_backtrace).

Sample Usage:

=end

require 'test/unit'

class TestThrowable < Test::Unit::TestCase
  def test_throwable
    throwable = Class.new do
      include Throwable
      def self.exception *args; new *args; end
    end

    begin
      raise throwable, "this must be handled"
      assert(false)
    rescue Exception
      assert(false)
    rescue Throwable
      assert(true)
    end
  end

  class SomeLibraryFromSomebodyElse
    def do_it
      something_that_takes_too_long
    rescue ::Exception
      $stderr.puts "Something failed"
      42
    end

    def something_that_takes_too_long
      sleep 10
    end
  end

  class MyTimeoutError
    include Throwable
    def self.exception *args; new *args; end
  end

  def test_throwable_timeout
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

