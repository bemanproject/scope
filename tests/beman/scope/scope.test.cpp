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


TEST_CASE("scope_guard")
{
    SECTION("Constructing")
    {
        SECTION("lambdas")
        {
            auto exit_guard0 = beman::scope::scope_guard{[] {}};

            auto exit_guard1 = beman::scope::scope_guard { [] {},
                                                           []
                                                           {
                                                               return true;
                                                           } };

            // vvv doesn't compile (as planned)- guard NOT releasable
            // exit_guard1.release();
            // ^^^

            auto exit_guard2 = beman::scope::scope_guard { [] {},
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

            } exit_func_onj;

            SECTION("func obj is called multiple times")
            {
                {
                    auto exit_guard3_1 = beman::scope::scope_guard { exit_func_onj };
                    auto exit_guard3_2 = beman::scope::scope_guard { exit_func_onj };
                    auto exit_guard3_3 = beman::scope::scope_guard { exit_func_onj };
                    auto exit_guard3_4 = beman::scope::scope_guard { exit_func_onj };
                    auto exit_guard3_5 = beman::scope::scope_guard { exit_func_onj };
                }

                REQUIRE(exit_func_onj.invoked_count == 5);
            }

            beman::scope::Releasable<> releaser;

            SECTION("Multiple exit guards disabled by releaser")
            {
                {
                    auto exit_guard3_1 = beman::scope::scope_guard { exit_func_onj, releaser };
                    auto exit_guard3_2 = beman::scope::scope_guard { exit_func_onj, releaser };
                    auto exit_guard3_3 = beman::scope::scope_guard { exit_func_onj, releaser };
                    auto exit_guard3_4 = beman::scope::scope_guard { exit_func_onj, releaser };
                    auto exit_guard3_5 = beman::scope::scope_guard { exit_func_onj, releaser };

                    releaser.release();
                }

                REQUIRE(exit_func_onj.invoked_count == 0);
            }
        }
    }

    SECTION("Moving - double release check")
    {
        struct FunctionObject
        {
            void operator()()
            {
                invoked_count++;
            }

            int invoked_count = 0;

        } exit_func_onj;


        auto guard = beman::scope::scope_guard { exit_func_onj };

        // auto guard2(std::move(guard));

        REQUIRE(exit_func_onj.invoked_count == 1);
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

        auto exit_guard5 = beman::scope::scope_guard { [] {},
                                                       []
                                                       {
                                                           return true;
                                                       } };

        // exit_guard5.release(); // doesn't compile (as planned)- NOT releasable

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

        auto exit_gaurd5 = beman::scope::scope_guard { [] {},
                                                       []
                                                       {
                                                           return true;
                                                       } };

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
