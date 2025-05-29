#pragma once
#include "HalfMove.hpp"

struct Move {
   HalfMove from;
   HalfMove to;

   Move(HalfMove from, HalfMove to) : from(from), to(to) {}
};