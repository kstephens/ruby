# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = %q{psych}
  s.version = "1.2.1"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = [%q{Aaron Patterson}]
  s.date = %q{2011-08-24}
  s.description = %q{Psych is a YAML parser and emitter.  Psych leverages libyaml[http://libyaml.org]
for its YAML parsing and emitting capabilities.  In addition to wrapping
libyaml, Psych also knows how to serialize and de-serialize most Ruby objects
to and from the YAML format.}
  s.email = [%q{aaron@tenderlovemaking.com}]
  s.extensions = [%q{ext/psych/extconf.rb}]
  s.files = [%q{.autotest}, %q{CHANGELOG.rdoc}, %q{Manifest.txt}, %q{README.rdoc}, %q{Rakefile}, %q{ext/psych/emitter.c}, %q{ext/psych/emitter.h}, %q{ext/psych/extconf.rb}, %q{ext/psych/parser.c}, %q{ext/psych/parser.h}, %q{ext/psych/psych.c}, %q{ext/psych/psych.h}, %q{ext/psych/to_ruby.c}, %q{ext/psych/to_ruby.h}, %q{ext/psych/yaml_tree.c}, %q{ext/psych/yaml_tree.h}, %q{lib/psych.rb}, %q{lib/psych/coder.rb}, %q{lib/psych/core_ext.rb}, %q{lib/psych/deprecated.rb}, %q{lib/psych/handler.rb}, %q{lib/psych/json.rb}, %q{lib/psych/json/ruby_events.rb}, %q{lib/psych/json/stream.rb}, %q{lib/psych/json/tree_builder.rb}, %q{lib/psych/json/yaml_events.rb}, %q{lib/psych/nodes.rb}, %q{lib/psych/nodes/alias.rb}, %q{lib/psych/nodes/document.rb}, %q{lib/psych/nodes/mapping.rb}, %q{lib/psych/nodes/node.rb}, %q{lib/psych/nodes/scalar.rb}, %q{lib/psych/nodes/sequence.rb}, %q{lib/psych/nodes/stream.rb}, %q{lib/psych/omap.rb}, %q{lib/psych/parser.rb}, %q{lib/psych/scalar_scanner.rb}, %q{lib/psych/set.rb}, %q{lib/psych/stream.rb}, %q{lib/psych/streaming.rb}, %q{lib/psych/tree_builder.rb}, %q{lib/psych/visitors.rb}, %q{lib/psych/visitors/depth_first.rb}, %q{lib/psych/visitors/emitter.rb}, %q{lib/psych/visitors/json_tree.rb}, %q{lib/psych/visitors/to_ruby.rb}, %q{lib/psych/visitors/visitor.rb}, %q{lib/psych/visitors/yaml_tree.rb}, %q{test/psych/helper.rb}, %q{test/psych/json/test_stream.rb}, %q{test/psych/nodes/test_enumerable.rb}, %q{test/psych/test_alias_and_anchor.rb}, %q{test/psych/test_array.rb}, %q{test/psych/test_boolean.rb}, %q{test/psych/test_class.rb}, %q{test/psych/test_coder.rb}, %q{test/psych/test_date_time.rb}, %q{test/psych/test_deprecated.rb}, %q{test/psych/test_document.rb}, %q{test/psych/test_emitter.rb}, %q{test/psych/test_encoding.rb}, %q{test/psych/test_engine_manager.rb}, %q{test/psych/test_exception.rb}, %q{test/psych/test_hash.rb}, %q{test/psych/test_json_tree.rb}, %q{test/psych/test_merge_keys.rb}, %q{test/psych/test_nil.rb}, %q{test/psych/test_null.rb}, %q{test/psych/test_object.rb}, %q{test/psych/test_omap.rb}, %q{test/psych/test_parser.rb}, %q{test/psych/test_psych.rb}, %q{test/psych/test_scalar.rb}, %q{test/psych/test_scalar_scanner.rb}, %q{test/psych/test_serialize_subclasses.rb}, %q{test/psych/test_set.rb}, %q{test/psych/test_stream.rb}, %q{test/psych/test_string.rb}, %q{test/psych/test_struct.rb}, %q{test/psych/test_symbol.rb}, %q{test/psych/test_tainted.rb}, %q{test/psych/test_to_yaml_properties.rb}, %q{test/psych/test_tree_builder.rb}, %q{test/psych/test_yaml.rb}, %q{test/psych/visitors/test_depth_first.rb}, %q{test/psych/visitors/test_emitter.rb}, %q{test/psych/visitors/test_to_ruby.rb}, %q{test/psych/visitors/test_yaml_tree.rb}, %q{.gemtest}]
  s.homepage = %q{http://github.com/tenderlove/psych}
  s.rdoc_options = [%q{--main}, %q{README.rdoc}]
  s.require_paths = [%q{lib}]
  s.required_ruby_version = Gem::Requirement.new(">= 1.9.2")
  s.rubyforge_project = %q{psych}
  s.rubygems_version = %q{1.8.8}
  s.summary = %q{Psych is a YAML parser and emitter}
  s.test_files = [%q{test/psych/json/test_stream.rb}, %q{test/psych/nodes/test_enumerable.rb}, %q{test/psych/test_alias_and_anchor.rb}, %q{test/psych/test_array.rb}, %q{test/psych/test_boolean.rb}, %q{test/psych/test_class.rb}, %q{test/psych/test_coder.rb}, %q{test/psych/test_date_time.rb}, %q{test/psych/test_deprecated.rb}, %q{test/psych/test_document.rb}, %q{test/psych/test_emitter.rb}, %q{test/psych/test_encoding.rb}, %q{test/psych/test_engine_manager.rb}, %q{test/psych/test_exception.rb}, %q{test/psych/test_hash.rb}, %q{test/psych/test_json_tree.rb}, %q{test/psych/test_merge_keys.rb}, %q{test/psych/test_nil.rb}, %q{test/psych/test_null.rb}, %q{test/psych/test_object.rb}, %q{test/psych/test_omap.rb}, %q{test/psych/test_parser.rb}, %q{test/psych/test_psych.rb}, %q{test/psych/test_scalar.rb}, %q{test/psych/test_scalar_scanner.rb}, %q{test/psych/test_serialize_subclasses.rb}, %q{test/psych/test_set.rb}, %q{test/psych/test_stream.rb}, %q{test/psych/test_string.rb}, %q{test/psych/test_struct.rb}, %q{test/psych/test_symbol.rb}, %q{test/psych/test_tainted.rb}, %q{test/psych/test_to_yaml_properties.rb}, %q{test/psych/test_tree_builder.rb}, %q{test/psych/test_yaml.rb}, %q{test/psych/visitors/test_depth_first.rb}, %q{test/psych/visitors/test_emitter.rb}, %q{test/psych/visitors/test_to_ruby.rb}, %q{test/psych/visitors/test_yaml_tree.rb}]
end
