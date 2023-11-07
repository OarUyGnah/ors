
target("raft-server")
    set_kind("binary")
    -- add_headerfiles("include/*.h")
    add_includedirs("../include")
    add_files("utils/*.cc")
    add_files("*.cc")
    add_syslinks("pthread")
    -- set_config("buildir", "build.xmake")