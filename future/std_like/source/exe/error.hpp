#pragma once
#include <exception>

struct NoStateError : public std::exception {};

struct PromiseAlreadySatisfiedError : public std::exception {};

struct BrokenPromiseError : public std::exception {};
