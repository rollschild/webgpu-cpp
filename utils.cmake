function(target_copy_webgpu_binaries Target)
  add_custom_command(
    TARGET ${Target}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WGPU_RUNTIME_LIB}
            $<TARGET_FILE_DIR:${Target}>)
endfunction()
