#include <gtest/gtest.h>
#include "Fizzbuzz.hpp"

TEST(FizzBuzz, ReturnTest)
{
    ASSERT_EQ(Fizzbuzz::fizzbuzz(1), "1");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(2), "2");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(4), "4");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(7), "7");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(101), "101");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(998), "998");
}

TEST(FizzBuzz, FizzTest)
{
    ASSERT_EQ(Fizzbuzz::fizzbuzz(3), "Fizz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(6), "Fizz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(12), "Fizz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(36), "Fizz");  
    ASSERT_EQ(Fizzbuzz::fizzbuzz(111), "Fizz");
}

TEST(FizzBuzz, BuzzTest)
{
    ASSERT_EQ(Fizzbuzz::fizzbuzz(5), "Buzz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(10), "Buzz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(20), "Buzz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(50), "Buzz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(500), "Buzz");
}

TEST(FizzBuzz, FizzBuzzTest)
{
    ASSERT_EQ(Fizzbuzz::fizzbuzz(0), "FizzBuzz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(15), "FizzBuzz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(30), "FizzBuzz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(45), "FizzBuzz");
    ASSERT_EQ(Fizzbuzz::fizzbuzz(600), "FizzBuzz");
}