add_requires("spdlog","protobuf","pqxx","pq")

target("main")
    set_kind("binary")
    -- add_headerfiles("include/*.h")
    add_includedirs("../include")
    add_files("utils/*.cc")
    add_files("*.cc")
    add_syslinks("pthread")
    
    add_packages("spdlog","protobuf","pqxx","pq")
    -- add_links("spdlog")
    -- set_config("buildir", "build.xmake")