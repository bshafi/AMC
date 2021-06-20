#pragma once

#include <set>

struct Testing {
    static std::set<void (*)()> & tests() {
        static std::set<void (*)()> tests;
        return tests;
    }
    static void register_test(void (*new_test)()) {
        tests().insert(new_test);
    }
};

struct TestRegisterer {
    TestRegisterer(void (*new_test)()) {
        Testing::register_test(new_test);
    }
};

#define REGISTER_TEST(Fn) struct Fn##RegisterClass { static TestRegisterer test_registerer; }; TestRegisterer Fn##RegisterClass::test_registerer = TestRegisterer(Fn); void Fn ()
