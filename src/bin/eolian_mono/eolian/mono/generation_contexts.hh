#ifndef EOLIAN_MONO_GENERATION_CONTEXTS_HH
#define EOLIAN_MONO_GENERATION_CONTEXTS_HH

namespace eolian_mono {

struct class_context
{
    enum wrapper_kind {
        interface,
        concrete,
        inherit,
        inherit_native,
        structs,
        function_ptr,
        alias,
    };
    wrapper_kind current_wrapper_kind;
};

struct library_context
{
  std::string library_name;
  int v_major;
  int v_minor;
  std::map<const std::string, std::string> references;

  const std::string actual_library_name(const std::string& filename) const;
};

const std::string
library_context::actual_library_name(const std::string& filename) const
{
    // Libraries mapped follow the efl.Libs.NAME scheme.
    // TODO What about references from outside efl (not present in the efl.Libs class?)
    auto ref = references.find(filename);
    if (ref != references.end())
      return "efl.Libs." + ref->second;

    // Fallback to original behaviour with explicit library name
    return '"' + library_name + '"';
}

}

#endif
