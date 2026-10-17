#include "owl_sudoku/sudoku.h"

#include <algorithm>
#include <array>
#include <bit>
#include <numeric>

namespace owl_sudoku
{
	namespace
	{
		constexpr int AllDigitsMask = 0x3FE; // bits 1..9
		constexpr int MaxGenerationAttempts = 4;

		struct ClueSpec
		{
			int target;
			int maxAllowed;
		};

		ClueSpec clueSpec(Difficulty difficulty)
		{
			switch (difficulty)
			{
			case Difficulty::Easy:   return { 40, 46 };
			case Difficulty::Medium: return { 34, 40 };
			case Difficulty::Hard:   return { 30, 36 };
			case Difficulty::Master: return { 26, 32 };
			}

			return { 40, 46 };
		}

		std::array<int, BoardSize> shuffledLines(std::mt19937& rng)
		{
			std::array<int, BoxSize> groups{ 0, 1, 2 };
			std::shuffle(groups.begin(), groups.end(), rng);

			std::array<int, BoardSize> order{};
			int next = 0;

			for (const int group : groups)
			{
				std::array<int, BoxSize> lines{ 0, 1, 2 };
				std::shuffle(lines.begin(), lines.end(), rng);

				for (const int line : lines)
				{
					order[next++] = group * BoxSize + line;
				}
			}

			return order;
		}

		Grid makeSolution(std::mt19937& rng)
		{
			std::array<int, BoardSize> digits{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
			std::shuffle(digits.begin(), digits.end(), rng);

			const std::array<int, BoardSize> rowOrder = shuffledLines(rng);
			const std::array<int, BoardSize> colOrder = shuffledLines(rng);

			Grid solution{};

			for (int r = 0; r < BoardSize; ++r)
			{
				for (int c = 0; c < BoardSize; ++c)
				{
					const int base = (rowOrder[r] * 3 + rowOrder[r] / 3 + colOrder[c]) % 9;
					solution[r][c] = digits[base];
				}
			}

			return solution;
		}

		Puzzle carvePuzzle(Difficulty difficulty, std::mt19937& rng)
		{
			const ClueSpec spec = clueSpec(difficulty);
			Puzzle best{};
			int bestClues = BoardSize * BoardSize + 1;

			for (int attempt = 0; attempt < MaxGenerationAttempts; ++attempt)
			{
				Puzzle puzzle;
				puzzle.solution = makeSolution(rng);
				puzzle.givens = puzzle.solution;

				std::array<int, BoardSize * BoardSize> positions{};
				std::iota(positions.begin(), positions.end(), 0);
				std::shuffle(positions.begin(), positions.end(), rng);

				int clues = BoardSize * BoardSize;

				for (const int position : positions)
				{
					if (clues <= spec.target)
					{
						break;
					}

					const int r = position / BoardSize;
					const int c = position % BoardSize;
					const int value = puzzle.givens[r][c];
					puzzle.givens[r][c] = 0;

					if (countSolutions(puzzle.givens, 2) == 1)
					{
						--clues;
					}
					else
					{
						puzzle.givens[r][c] = value;
					}
				}

				if (clues <= spec.maxAllowed)
				{
					return puzzle;
				}

				if (clues < bestClues)
				{
					bestClues = clues;
					best = puzzle;
				}
			}

			return best;
		}


		int boxIndex(int r, int c)
		{
			return (r / BoxSize) * BoxSize + (c / BoxSize);
		}

		int searchSolutions(Grid& grid,
			std::array<int, BoardSize>& rowMask,
			std::array<int, BoardSize>& colMask,
			std::array<int, BoardSize>& boxMask,
			int remaining,
			int limit)
		{
			if (remaining == 0)
			{
				return 1;
			}

			int bestRow = -1;
			int bestCol = -1;
			int bestCandidates = 0;
			int fewest = BoardSize + 1;

			for (int r = 0; r < BoardSize; ++r)
			{
				for (int c = 0; c < BoardSize; ++c)
				{
					if (grid[r][c] != 0)
					{
						continue;
					}

					const int used = rowMask[r] | colMask[c] | boxMask[boxIndex(r, c)];
					const int candidates = (~used) & AllDigitsMask;
					const int count = std::popcount(static_cast<unsigned>(candidates));

					if (count == 0)
					{
						return 0;
					}

					if (count < fewest)
					{
						fewest = count;
						bestRow = r;
						bestCol = c;
						bestCandidates = candidates;
					}
				}
			}

			if (bestRow < 0)
			{
				return 0;
			}

			const int box = boxIndex(bestRow, bestCol);
			int total = 0;

			for (int value = 1; value <= BoardSize; ++value)
			{
				const int bit = 1 << value;

				if ((bestCandidates & bit) == 0)
				{
					continue;
				}

				grid[bestRow][bestCol] = value;
				rowMask[bestRow] |= bit;
				colMask[bestCol] |= bit;
				boxMask[box] |= bit;

				total += searchSolutions(grid, rowMask, colMask, boxMask, remaining - 1, limit);

				grid[bestRow][bestCol] = 0;
				rowMask[bestRow] &= ~bit;
				colMask[bestCol] &= ~bit;
				boxMask[box] &= ~bit;

				if (total >= limit)
				{
					return total;
				}
			}

			return total;
		}
	}

	int countSolutions(const Grid& grid, int limit)
	{
		if (limit <= 0)
		{
			return 0;
		}

		Grid work = grid;
		std::array<int, BoardSize> rowMask{};
		std::array<int, BoardSize> colMask{};
		std::array<int, BoardSize> boxMask{};
		int remaining = 0;

		for (int r = 0; r < BoardSize; ++r)
		{
			for (int c = 0; c < BoardSize; ++c)
			{
				const int value = work[r][c];

				if (value == 0)
				{
					++remaining;
					continue;
				}

				if (value < 1 || value > 9)
				{
					return 0;
				}

				const int bit = 1 << value;
				const int box = boxIndex(r, c);

				if ((rowMask[r] & bit) || (colMask[c] & bit) || (boxMask[box] & bit))
				{
					return 0;
				}

				rowMask[r] |= bit;
				colMask[c] |= bit;
				boxMask[box] |= bit;
			}
		}

		return searchSolutions(work, rowMask, colMask, boxMask, remaining, limit);
	}

	std::optional<Cell> cellAt(double localX, double localY, double cellSize)
	{
		if ((cellSize <= 0.0) || (localX < 0.0) || (localY < 0.0))
		{
			return std::nullopt;
		}

		const double boardPixels = cellSize * BoardSize;

		if ((localX >= boardPixels) || (localY >= boardPixels))
		{
			return std::nullopt;
		}

		return Cell{
			static_cast<int>(localY / cellSize),
			static_cast<int>(localX / cellSize),
		};
	}

	Cell moveCell(Cell cell, int rowDelta, int columnDelta)
	{
		return Cell{
			std::clamp(cell.row + rowDelta, 0, BoardSize - 1),
			std::clamp(cell.column + columnDelta, 0, BoardSize - 1),
		};
	}

	Puzzle generate(Difficulty difficulty, std::mt19937& rng)
	{
		return carvePuzzle(difficulty, rng);
	}

	Puzzle generate(Difficulty difficulty)
	{
		std::random_device randomDevice;
		std::mt19937 rng{ randomDevice() };
		return generate(difficulty, rng);
	}

	bool isValidSolution(const Grid& grid)
	{
		std::array<int, BoardSize> rowMask{};
		std::array<int, BoardSize> colMask{};
		std::array<int, BoardSize> boxMask{};

		for (int r = 0; r < BoardSize; ++r)
		{
			for (int c = 0; c < BoardSize; ++c)
			{
				const int value = grid[r][c];

				if (value < 1 || value > 9)
				{
					return false;
				}

				const int bit = 1 << value;
				const int box = (r / BoxSize) * BoxSize + (c / BoxSize);

				if ((rowMask[r] & bit) || (colMask[c] & bit) || (boxMask[box] & bit))
				{
					return false;
				}

				rowMask[r] |= bit;
				colMask[c] |= bit;
				boxMask[box] |= bit;
			}
		}

		return true;
	}

	bool isComplete(const Grid& grid)
	{
		for (int r = 0; r < BoardSize; ++r)
		{
			for (int c = 0; c < BoardSize; ++c)
			{
				if (grid[r][c] == 0)
				{
					return false;
				}
			}
		}

		return isValidSolution(grid);
	}

	bool hasConflict(const Grid& grid, int row, int column)
	{
		if ((row < 0) || (row >= BoardSize) || (column < 0) || (column >= BoardSize))
		{
			return false;
		}

		const int value = grid[row][column];

		if (value == 0)
		{
			return false;
		}

		for (int c = 0; c < BoardSize; ++c)
		{
			if ((c != column) && (grid[row][c] == value))
			{
				return true;
			}
		}

		for (int r = 0; r < BoardSize; ++r)
		{
			if ((r != row) && (grid[r][column] == value))
			{
				return true;
			}
		}

		const int boxRow = (row / BoxSize) * BoxSize;
		const int boxColumn = (column / BoxSize) * BoxSize;

		for (int r = boxRow; r < boxRow + BoxSize; ++r)
		{
			for (int c = boxColumn; c < boxColumn + BoxSize; ++c)
			{
				if (((r != row) || (c != column)) && (grid[r][c] == value))
				{
					return true;
				}
			}
		}

		return false;
	}
}
