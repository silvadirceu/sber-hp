# LIVE-ID
add_executable(live-id live-id.cpp)

target_compile_features(live-id PRIVATE cxx_std_20)

target_link_libraries(live-id PRIVATE hpfw)


# COMBINE
add_executable(combine combine.cpp)

target_compile_features(combine PRIVATE cxx_std_20)

target_link_libraries(combine PRIVATE hpfw)
