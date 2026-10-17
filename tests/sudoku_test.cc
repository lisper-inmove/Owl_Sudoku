#include <gtest/gtest.h>
#include "owl_sudoku/sudoku.h"

namespace
{
	using owl_sudoku::Grid;

	Grid completeGrid()
	{
		return Grid{ {
			{{5, 3, 4, 6, 7, 8, 9, 1, 2}},
			{{6, 7, 2, 1, 9, 5, 3, 4, 8}},
			{{1, 9, 8, 3, 4, 2, 5, 6, 7}},
			{{8, 5, 9, 7, 6, 1, 4, 2, 3}},
			{{4, 2, 6, 8, 5, 3, 7, 9, 1}},
			{{7, 1, 3, 9, 2, 4, 8, 5, 6}},
			{{9, 6, 1, 5, 3, 7, 2, 8, 4}},
			{{2, 8, 7, 4, 1, 9, 6, 3, 5}},
			{{3, 4, 5, 2, 8, 6, 1, 7, 9}},
		} };
	}
}

TEST(SudokuValidation, CompleteSolutionIsValidAndComplete)
{
	const Grid grid = completeGrid();
	EXPECT_TRUE(owl_sudoku::isValidSolution(grid));
	EXPECT_TRUE(owl_sudoku::isComplete(grid));
}

TEST(SudokuValidation, GridWithZeroIsNeitherValidNorComplete)
{
	Grid grid = completeGrid();
	grid[0][0] = 0;
	EXPECT_FALSE(owl_sudoku::isValidSolution(grid));
	EXPECT_FALSE(owl_sudoku::isComplete(grid));
}

TEST(SudokuValidation, DuplicateInRowIsInvalid)
{
	Grid grid = completeGrid();
	grid[0][1] = 5;
	EXPECT_FALSE(owl_sudoku::isValidSolution(grid));
	EXPECT_FALSE(owl_sudoku::isComplete(grid));
}

TEST(SudokuValidation, OutOfRangeValueIsInvalid)
{
	Grid grid = completeGrid();
	grid[0][0] = 10;
	EXPECT_FALSE(owl_sudoku::isValidSolution(grid));
	EXPECT_FALSE(owl_sudoku::isComplete(grid));
}

namespace
{
	Grid classicPuzzle()
	{
		return Grid{ {
			{{5, 3, 0, 0, 7, 0, 0, 0, 0}},
			{{6, 0, 0, 1, 9, 5, 0, 0, 0}},
			{{0, 9, 8, 0, 0, 0, 0, 6, 0}},
			{{8, 0, 0, 0, 6, 0, 0, 0, 3}},
			{{4, 0, 0, 8, 0, 3, 0, 0, 1}},
			{{7, 0, 0, 0, 2, 0, 0, 0, 6}},
			{{0, 6, 0, 0, 0, 0, 2, 8, 0}},
			{{0, 0, 0, 4, 1, 9, 0, 0, 5}},
			{{0, 0, 0, 0, 8, 0, 0, 7, 9}},
		} };
	}
}

TEST(SudokuSolver, EmptyGridReachesLimit)
{
	const Grid empty{};
	EXPECT_EQ(owl_sudoku::countSolutions(empty, 1), 1);
	EXPECT_EQ(owl_sudoku::countSolutions(empty, 2), 2);
}

TEST(SudokuSolver, ClassicPuzzleHasUniqueSolution)
{
	EXPECT_EQ(owl_sudoku::countSolutions(classicPuzzle(), 2), 1);
}

TEST(SudokuSolver, DuplicateGridHasNoSolution)
{
	Grid grid = classicPuzzle();
	grid[0][2] = 5; // 与 grid[0][0] 冲突
	EXPECT_EQ(owl_sudoku::countSolutions(grid, 2), 0);
}

TEST(SudokuSolver, CompleteSolutionHasExactlyOneSolution)
{
	EXPECT_EQ(owl_sudoku::countSolutions(completeGrid(), 2), 1);
}

TEST(SudokuGenerator, GeneratesValidUniquePuzzlesForEveryDifficulty)
{
	struct Case
	{
		owl_sudoku::Difficulty difficulty;
		int minClues;
		int maxClues;
	};

	const Case cases[] = {
		{ owl_sudoku::Difficulty::Easy,   40, 46 },
		{ owl_sudoku::Difficulty::Medium, 34, 40 },
		{ owl_sudoku::Difficulty::Hard,   30, 36 },
		{ owl_sudoku::Difficulty::Master, 26, 32 },
	};

	for (const Case& c : cases)
	{
		for (const unsigned seed : { 123u, 456u })
		{
			std::mt19937 rng{ seed };
			const owl_sudoku::Puzzle puzzle = owl_sudoku::generate(c.difficulty, rng);

			EXPECT_TRUE(owl_sudoku::isValidSolution(puzzle.solution));
			EXPECT_EQ(owl_sudoku::countSolutions(puzzle.givens, 2), 1);

			int clues = 0;

			for (int r = 0; r < owl_sudoku::BoardSize; ++r)
			{
				for (int col = 0; col < owl_sudoku::BoardSize; ++col)
				{
					if (puzzle.givens[r][col] != 0)
					{
						++clues;
						EXPECT_EQ(puzzle.givens[r][col], puzzle.solution[r][col]);
					}
				}
			}

			EXPECT_GE(clues, c.minClues);
			EXPECT_LE(clues, c.maxClues);
		}
	}
}

TEST(SudokuGenerator, SameSeedProducesSamePuzzle)
{
	std::mt19937 rngA{ 777u };
	std::mt19937 rngB{ 777u };
	const auto a = owl_sudoku::generate(owl_sudoku::Difficulty::Hard, rngA);
	const auto b = owl_sudoku::generate(owl_sudoku::Difficulty::Hard, rngB);
	EXPECT_EQ(a.givens, b.givens);
	EXPECT_EQ(a.solution, b.solution);
}

TEST(SudokuGenerator, RandomOverloadReturnsUniquePuzzle)
{
	const auto puzzle = owl_sudoku::generate(owl_sudoku::Difficulty::Easy);
	EXPECT_TRUE(owl_sudoku::isValidSolution(puzzle.solution));
	EXPECT_EQ(owl_sudoku::countSolutions(puzzle.givens, 2), 1);
}

TEST(SudokuGeometry, CellAtMapsLocalCoordinatesToRowAndColumn)
{
	const auto topLeft = owl_sudoku::cellAt(30.0, 30.0, 60.0);
	ASSERT_TRUE(topLeft.has_value());
	EXPECT_EQ(topLeft->row, 0);
	EXPECT_EQ(topLeft->column, 0);

	const auto offDiagonal = owl_sudoku::cellAt(270.0, 90.0, 60.0);
	ASSERT_TRUE(offDiagonal.has_value());
	EXPECT_EQ(offDiagonal->row, 1);
	EXPECT_EQ(offDiagonal->column, 4);

	const auto bottomRight = owl_sudoku::cellAt(539.0, 539.0, 60.0);
	ASSERT_TRUE(bottomRight.has_value());
	EXPECT_EQ(bottomRight->row, 8);
	EXPECT_EQ(bottomRight->column, 8);

	EXPECT_FALSE(owl_sudoku::cellAt(-1.0, 0.0, 60.0).has_value());
	EXPECT_FALSE(owl_sudoku::cellAt(0.0, -1.0, 60.0).has_value());
	EXPECT_FALSE(owl_sudoku::cellAt(540.0, 0.0, 60.0).has_value());
	EXPECT_FALSE(owl_sudoku::cellAt(0.0, 540.0, 60.0).has_value());
}

TEST(SudokuGeometry, MoveCellUsesRowColumnDeltasAndClamps)
{
	EXPECT_EQ(owl_sudoku::moveCell({ 4, 4 }, 0, 1), (owl_sudoku::Cell{ 4, 5 }));
	EXPECT_EQ(owl_sudoku::moveCell({ 4, 4 }, 1, 0), (owl_sudoku::Cell{ 5, 4 }));
	EXPECT_EQ(owl_sudoku::moveCell({ 0, 0 }, -1, 0), (owl_sudoku::Cell{ 0, 0 }));
	EXPECT_EQ(owl_sudoku::moveCell({ 8, 8 }, 1, 1), (owl_sudoku::Cell{ 8, 8 }));
}

TEST(SudokuConflict, NonConflictingWrongValueIsNotFlagged)
{
	Grid grid = classicPuzzle();
	grid[0][2] = 1; // 解法为 4，但行、列、宫都没有 1
	EXPECT_FALSE(owl_sudoku::hasConflict(grid, 0, 2));
}

TEST(SudokuConflict, DetectsRowColumnAndBoxConflicts)
{
	Grid rowOnly = classicPuzzle();
	rowOnly[0][3] = 3; // 与 (0,1) 的 3 同行
	EXPECT_TRUE(owl_sudoku::hasConflict(rowOnly, 0, 3));

	Grid columnOnly = classicPuzzle();
	columnOnly[0][3] = 4; // 与 (7,3) 的 4 同列
	EXPECT_TRUE(owl_sudoku::hasConflict(columnOnly, 0, 3));

	Grid boxOnly = classicPuzzle();
	boxOnly[0][2] = 6; // 与 (1,0) 的 6 同宫
	EXPECT_TRUE(owl_sudoku::hasConflict(boxOnly, 0, 2));
}

TEST(SudokuConflict, EmptyAndCompleteGridsHaveNoConflicts)
{
	const Grid empty{};
	EXPECT_FALSE(owl_sudoku::hasConflict(empty, 0, 0));

	const Grid complete = completeGrid();
	EXPECT_FALSE(owl_sudoku::hasConflict(complete, 0, 0));
	EXPECT_FALSE(owl_sudoku::hasConflict(complete, 8, 8));
}
