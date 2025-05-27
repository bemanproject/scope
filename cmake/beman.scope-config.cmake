include(${CMAKE_CURRENT_LIST_DIR}/beman.scope-targets.cmake)

foreach(comp IN LISTS beman.scope_FIND_COMPONENTS)
    if(beman.scope_FIND_REQUIRED_${comp})
        set(beman.scope_FOUND FALSE)
        return()
    endif()
endforeach()
