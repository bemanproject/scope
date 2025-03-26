// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/scope/scope.hpp>

// define only in one cpp file
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

TEST_CASE("concepts")
{
    using namespace beman::scope;

    REQUIRE(HasDontInvokeOnCreationException<ExecuteWhenNoException>);

    REQUIRE_FALSE(HasDontInvokeOnCreationException<ExecuteOnlyWhenException>);
    REQUIRE_FALSE(HasDontInvokeOnCreationException<ExecuteAlways>);
}


namespace scope_testing {

static bool is_executed = false;


static void global_function()
{
    is_executed = true;
}


}

TEST_CASE("scope_guard")
{
    SECTION("Constructing")
    {
        SECTION("lambdas")
        {
            auto exit_guard0 = beman::scope::scope_guard{[] {}};
            //beman::scope::scope_guard exit_guard0 {[] {}};

            // vvv doesn't compile (as planned)- guard NOT releasable
            // exit_guard0.release();
            // ^^^

            auto exit_guard_with_bool_checker = beman::scope::scope_guard { [] {},
                                                                            []
                                                                            {
                                                                                return true;
                                                                            } };


            auto exit_guard_with_convertable_to_bool_checker = beman::scope::scope_guard { [] {},
                                                                                           []
                                                                                           {
                                                                                               return 1;
                                                                                           } };

            REQUIRE(true);
        }

        SECTION("Function object")
        {
            struct FunctionObject
            {
                void operator()()
                {
                    invoked_count++;
                }

                int invoked_count = 0;

            } exit_func_obj;

            SECTION("func obj is called multiple times")
            {
                {
                    auto exit_guard3_1 = beman::scope::scope_guard { std::ref(exit_func_obj) };
                    auto exit_guard3_2 = beman::scope::scope_guard { std::ref(exit_func_obj) };
                    auto exit_guard3_3 = beman::scope::scope_guard { std::ref(exit_func_obj) };
                    auto exit_guard3_4 = beman::scope::scope_guard { std::ref(exit_func_obj) };
                    auto exit_guard3_5 = beman::scope::scope_guard { std::ref(exit_func_obj) };
                }

                REQUIRE(exit_func_obj.invoked_count == 5);
            }

            SECTION("Multiple exit guards disabled by 1 releaser")
            {
                {
                    beman::scope::Releasable<> releaser;

                    auto exit_guard3_1 = beman::scope::scope_guard { exit_func_obj, releaser };
                    auto exit_guard3_2 = beman::scope::scope_guard { exit_func_obj, releaser };
                    auto exit_guard3_3 = beman::scope::scope_guard { exit_func_obj, releaser };
                    auto exit_guard3_4 = beman::scope::scope_guard { exit_func_obj, releaser };
                    auto exit_guard3_5 = beman::scope::scope_guard { exit_func_obj, releaser };

                    releaser.release();
                }

                REQUIRE(exit_func_obj.invoked_count == 0);
            }


            SECTION("Multiple exit guards guarded by 1 releaser")
            {
                {
                    beman::scope::Releasable<> releaser;

                    auto exit_guard3_1 = beman::scope::scope_guard { std::ref(exit_func_obj), releaser };
                    auto exit_guard3_2 = beman::scope::scope_guard { std::ref(exit_func_obj), releaser };
                    auto exit_guard3_3 = beman::scope::scope_guard { std::ref(exit_func_obj), releaser };
                    auto exit_guard3_4 = beman::scope::scope_guard { std::ref(exit_func_obj), releaser };
                    auto exit_guard3_5 = beman::scope::scope_guard { std::ref(exit_func_obj), releaser };
                }

                REQUIRE(exit_func_obj.invoked_count == 5);
            }
        }

        SECTION("Function pointer")
        {
            {
                auto exit_guard = beman::scope::scope_guard { scope_testing::global_function };
            }

            REQUIRE(scope_testing::is_executed);
        }
    }

    SECTION("Moving - double release check")
    {
        int invoked_count = 0;

        {
            auto guard = beman::scope::scope_guard { [&] { ++invoked_count; }, beman::scope::Releasable<> {} };


            auto guard2(std::move(guard));
        }

        REQUIRE(invoked_count == 1);
    }
}


TEST_CASE("scope_exit")
{
    SECTION("Constructing")
    {
        beman::scope::scope_exit exit_guard1([] {});

        // beman::scope::scope_exit exit_guard2 = [] {};   // can't do: no conversion

        // beman::scope::scope_exit exit_guard3 = {[] {}}; // can't do: explict constructor

        auto exit_guard4 = beman::scope::scope_exit([] {});

        exit_guard4.release(); // scope_exit releasable by default.

        REQUIRE(true);
    }

    SECTION("Using")
    {
        SECTION("Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_exit exit_guard([&] { is_exit_func_invoked = true; });

                throw std::exception {};
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == true);
        }

        SECTION("NO Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_exit exit_guard([&] { is_exit_func_invoked = true; });
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE_FALSE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == true);
        }
    }
    SECTION("Using with release")
    {
        SECTION("Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_exit exit_guard([&] { is_exit_func_invoked = true; });

                exit_guard.release();

                throw std::exception {};
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == false);
        }

        SECTION("NO Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_exit exit_guard([&] { is_exit_func_invoked = true; });

                exit_guard.release();
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE_FALSE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == false);
        }
    }
}


TEST_CASE("scope_fail")
{
    SECTION("Constructing")
    {
        beman::scope::scope_fail exit_guard1([] {});

        // beman::scope::scope_fail exit_guard2 = [] {};   // can't do: no conversion

        // beman::scope::scope_fail exit_guard3 = {[] {}}; // can't do: explict constructor

        auto exit_guard4 = beman::scope::scope_fail([] {});

        exit_guard4.release(); // scope_fail releasable by default.

        REQUIRE(true);
    }

    SECTION("Using")
    {
        SECTION("Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_fail exit_guard([&] { is_exit_func_invoked = true; });

                throw std::exception {};
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == true);
        }

        SECTION("NO Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_fail exit_guard([&] { is_exit_func_invoked = true; });
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE_FALSE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == false);
        }
    }

    SECTION("Using with release")
    {
        SECTION("Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_fail exit_guard([&] { is_exit_func_invoked = true; });

                throw std::exception {};

                exit_guard.release();
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == true);
        }

        SECTION("NO Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_fail exit_guard([&] { is_exit_func_invoked = true; });

                exit_guard.release();
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE_FALSE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == false);
        }
    }
}


TEST_CASE("scope_success")
{
    SECTION("Constructing")
    {
        beman::scope::scope_success exit_guard1([] {});

        // beman::scope::scope_success exit_guard2 = [] {};   // can't do: no conversion

        // beman::scope::scope_success exit_guard3 = {[] {}}; // can't do: explict constructor

        auto exit_guard4 = beman::scope::scope_success([] {});

        exit_guard4.release(); // scope_success releasable by default

        REQUIRE(true);
    }

    SECTION("Using")
    {
        SECTION("Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_success exit_guard([&] { is_exit_func_invoked = true; });

                throw std::exception {};
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == false);
        }

        SECTION("NO Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_success exit_guard([&] { is_exit_func_invoked = true; });
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE_FALSE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == true);
        }
    }

    SECTION("Using with release")
    {
        SECTION("Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_success exit_guard([&] { is_exit_func_invoked = true; });

                exit_guard.release();

                throw std::exception {};
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == false);
        }

        SECTION("NO Exception thrown")
        {
            bool is_exception_thrown  = false;
            bool is_exit_func_invoked = false;

            try
            {
                beman::scope::scope_success exit_guard([&] { is_exit_func_invoked = true; });

                exit_guard.release();
            }
            catch (...)
            {
                is_exception_thrown = true;
            }

            REQUIRE_FALSE(is_exception_thrown);
            REQUIRE(is_exit_func_invoked == false);
        }
    }
}
