#pragma once

#include <cstdint>
#include <optional>
#include <string>

inline constexpr std::uint16_t GamePort = 12346;

enum class Move { Rock, Paper, Scissors };

inline std::optional<Move> ParseMove(const std::string& input)
{
    if (input == "1")
        return Move::Rock;
    if (input == "2")
        return Move::Paper;
    if (input == "3")
        return Move::Scissors;
    return std::nullopt;
}

inline bool Beats(Move first, Move second)
{
    return (first == Move::Rock && second == Move::Scissors)
        || (first == Move::Paper && second == Move::Rock)
        || (first == Move::Scissors && second == Move::Paper);
}
