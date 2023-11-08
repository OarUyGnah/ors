-- xmake -g test 构建整个test
set_group("test")
set_default(true)

add_requires("gflags", "glog","gtest","spdlog")
add_packages("gflags", "glog", "gtest","spdlog")
add_links("gtest_main")
add_syslinks("pthread")

if not is_plat("macosx") then
    add_ldflags("-lrt")
end
add_files("../src/utils/*.cc")
add_includedirs("../include")

function all_tests()
    local res = {}
    for _, x in ipairs(os.files("**.cc")) do
        local item = {}
        local s = path.filename(x)
        table.insert(item, s:sub(1, #s - 3)) -- target
        table.insert(item, path.relative(x, ".")) -- source
        table.insert(res, item)
    end
    return res
end

for _, test in ipairs(all_tests()) do
    target(test[1])
        set_kind("binary")
        add_files(test[2])
        if has_config("memcheck") then
            on_run(function (target)
                local argv = {}
                table.insert(argv, target:targetfile())
                table.insert(argv, "--leak-check=full")
                os.execv("valgrind", argv)
            end)
        end
end

-- add_requires("gtest")
-- 构建所有test xmake run all-test
target("all-test")
    set_kind("binary")
    -- add_packages("gtest")
    -- set_default(false)
    -- add_links("gtest_main")
    add_files("*.cc","../src/utils/*.cc")
    add_includedirs("../include")