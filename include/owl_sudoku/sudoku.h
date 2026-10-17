#pragma once

#include <array>
#include <optional>
#include <random>

namespace owl_sudoku
{
	constexpr int BoardSize = 9;
	constexpr int BoxSize = 3;

	// 0 表示空格；有效数字为 1..9。
	using Grid = std::array<std::array<int, BoardSize>, BoardSize>;

	struct Cell
	{
		int row = 0;
		int column = 0;

		friend bool operator ==(const Cell&, const Cell&) = default;
	};

	// 把棋盘左上角为原点的坐标映射到格子；越界返回 std::nullopt。
	std::optional<Cell> cellAt(double localX, double localY, double cellSize);

	// 按行/列增量移动格子，并限制在棋盘内。
	Cell moveCell(Cell cell, int rowDelta, int columnDelta);

	enum class Difficulty
	{
		Easy,
		Medium,
		Hard,
		Master,
	};

	struct Puzzle
	{
		Grid givens{};
		Grid solution{};
	};

	Puzzle generate(Difficulty difficulty, std::mt19937& rng);
	Puzzle generate(Difficulty difficulty);

	int countSolutions(const Grid& grid, int limit = 2);

	bool isValidSolution(const Grid& grid);
	bool isComplete(const Grid& grid);

	// 指定格子的数字是否与同行、同列或同宫的其他数字重复。
	bool hasConflict(const Grid& grid, int row, int column);
}
