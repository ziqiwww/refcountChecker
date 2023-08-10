add_library(RefcntAnalysis MODULE
        # List your source files here.
        analysis/RefcntAnalysis.cpp
        analysis/Fact.cpp
        analysis/AAHelper.cpp
        preproc/JsonParser.cpp
        printer/CFGDrawer.hpp
        )

target_compile_features(RefcntAnalysis PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(RefcntAnalysis PROPERTIES
        COMPILE_FLAGS "-fno-rtti"
        )

# include headers 
include_directories(./)
include_directories(../common)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if (APPLE)
    set_target_properties(RefcntAnalysis PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup"
            )
endif (APPLE)