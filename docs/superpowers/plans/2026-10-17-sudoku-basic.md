# Owl Sudoku 基础版 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在现有 Siv3D 0.6.16 模板工程中实现可选择四个难度、随机生成唯一解关卡、鼠标/键盘游玩并判定通关的数独游戏。

**Architecture:** 出题、求解与完成判定放在不依赖 Siv3D 的 `owl_sudoku` 核心模块中，由 GoogleTest 覆盖；`src/Main.cpp` 只负责 Siv3D 窗口、菜单、棋盘绘制与输入。生成器先构造随机完整解，再随机挖空并用“解数计数”保证唯一解。

**Tech Stack:** C++23、OpenSiv3D 0.6.16、CMake + Ninja Multi-Config、MSVC（静态 CRT）、GoogleTest。

**Spec:** `docs/superpowers/specs/2026-10-17-sudoku-basic-design.md`

## Global Constraints

- 仅使用 C++ 标准库与 Siv3D 0.6.16；不新增第三方依赖。
- `include/owl_sudoku/` 与 `src/sudoku.cc` 不得包含 Siv3D 头文件。
- 核心命名空间为 `owl_sudoku`；头文件使用 `#pragma once`。
- 窗口标题 `Owl Sudoku`，窗口尺寸 960×800。
- 中文 UI 使用 `Typeface::CJK_Regular_SC` 位图字体；数字可用 `Typeface::Bold`。
- 难度参数：简单目标 40 / 上限 46；中等 34 / 40；困难 30 / 36；大师 26 / 32。最多重试 4 次。
- 棋盘格边长 60，棋盘左上角 `(210, 130)`，总尺寸 540×540。
- 数字键为 `1`–`9` 与小键盘 `KeyNum1`–`KeyNum9`；清除键为 `Backspace` / `Delete`。
- 构建目标名保持 `Siv3D_Template` 与 `siv3d_template_tests` 不变。
- 按当前会话约定不自动执行 git commit；每个任务以构建/测试通过作为完成标志。

## Review Focus

1. **无选中格时按数字键**：必须无操作且不崩溃；由 Task 5 的手工检查覆盖。
2. **对题目给定格输入数字**：格子内容不得改变；由 Task 5 的手工检查覆盖。
3. **游戏中返回菜单或点“新游戏”后再开局**：选中格、通关遮罩、旧棋盘状态必须全部清空；由 Task 5 的手工检查覆盖。
4. **通关遮罩出现后点击棋盘区域**：不得修改棋盘；由 Task 5 的手工检查覆盖。
5. **生成器在不利随机序列下超出线索上限**：必须在 4 次尝试后仍返回合法且唯一解的题目，不得死循环；由 Task 3 的线索范围测试覆盖。

---

### Task 1: 核心类型、合法性检查与完成判定

**Files:**
- Create: `include/owl_sudoku/sudoku.h`
- Create: `src/sudoku.cc`
- Create: `tests/sudoku_test.cc`
- Modify: `CMakeLists.txt:88-92`（把 `src/sudoku.cc` 加入 `siv3d_template_core`）
- Modify: `CMakeLists.txt:213-218`（把 `tests/sudoku_test.cc` 加入 `siv3d_template_tests`）

**Interfaces:**
- Consumes: 无。
- Produces: `owl_sudoku::Grid`、`owl_sudoku::Difficulty`、`owl_sudoku::Puzzle`、`isValidSolution(const Grid&) -> bool`、`isComplete(const Grid&) -> bool`。同时声明 `countSolutions` 与两个 `generate` 重载，供后续任务实现。

- [ ] **Step 1: 建立头文件与失败的测试**

创建 `include/owl_sudoku/sudoku.h`，内容为：

```cpp
#pragma once

#include <array>
#include <random>

namespace owl_sudoku
{
	constexpr int BoardSize = 9;
	constexpr int BoxSize = 3;

	// 0 表示空格；有效数字为 1..9。
	using Grid = std::array<std::array<int, BoardSize>, BoardSize>;

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
}
```

创建 `tests/sudoku_test.cc`，使用经典数独终盘与题目：

```cpp
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
```

- [ ] **Step 2: 接入构建并确认测试失败**

修改 `CMakeLists.txt`：

```cmake
add_library(siv3d_template_core STATIC
	src/config.cc
	src/logging.cc
	src/sudoku.cc
	src/version.cc
)
```

```cmake
	add_executable(siv3d_template_tests
		tests/main.cpp
		tests/config_test.cc
		tests/logging_test.cc
		tests/sudoku_test.cc
		tests/version_test.cc
	)
```

先创建空的 `src/sudoku.cc`（仅 `#include "owl_sudoku/sudoku.h"`），然后运行：

```bash
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset debug"
```

Expected: 编译通过但测试可执行文件链接失败，报 `isValidSolution` / `isComplete` 未定义（unresolved external symbol）。

- [ ] **Step 3: 实现合法性检查与完成判定**

在 `src/sudoku.cc` 中实现：

- `isValidSolution(grid)`：遍历 81 格；出现 `0`、越界值或任一行/列/宫内重复时返回 `false`，否则 `true`。可用三个 `std::array<int, 9>` 位掩码（行、列、宫）实现。
- `isComplete(grid)`：先确认没有 `0`，再返回 `isValidSolution(grid)`。

- [ ] **Step 4: 运行测试确认通过**

```bash
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset debug"
./Intermediate/Siv3D_Template/Tests/Debug/siv3d_template_tests.exe --gtest_filter='SudokuValidation.*'
```

Expected: 4 个测试全部 PASS。

---

### Task 2: 解数计数

**Files:**
- Modify: `src/sudoku.cc`
- Modify: `tests/sudoku_test.cc`

**Interfaces:**
- Consumes: Task 1 的 `Grid`、`BoardSize`、`BoxSize`。
- Produces: `int countSolutions(const Grid& grid, int limit = 2)`。

- [ ] **Step 1: 写失败的测试**

在 `tests/sudoku_test.cc` 追加：

```cpp
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
```

- [ ] **Step 2: 确认测试失败**

```bash
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset debug"
```

Expected: 链接失败，`countSolutions` 未定义。

- [ ] **Step 3: 实现位掩码 + MRV 回溯计数**

在 `src/sudoku.cc` 中实现 `countSolutions`：

- 先校验输入：出现越界值或重复（行/列/宫）直接返回 `0`。
- 用 `rowMask[9]`、`colMask[9]`、`boxMask[9]` 记录已用数字位。
- 递归：每步扫描所有空格，选择候选数最少的格子；按候选位从低到高尝试。
- 每完成一个合法棋盘计数一次；计数达到 `limit` 立即返回。
- 全部分支结束后返回计数。

- [ ] **Step 4: 运行测试确认通过**

```bash
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset debug"
./Intermediate/Siv3D_Template/Tests/Debug/siv3d_template_tests.exe --gtest_filter='SudokuSolver.*'
```

Expected: 4 个测试全部 PASS。

---

### Task 3: 随机出题与难度

**Files:**
- Modify: `src/sudoku.cc`
- Modify: `tests/sudoku_test.cc`

**Interfaces:**
- Consumes: `countSolutions`、`isValidSolution`、`Grid`、`Difficulty`、`Puzzle`。
- Produces: `Puzzle generate(Difficulty difficulty, std::mt19937& rng)`、`Puzzle generate(Difficulty difficulty)`。

- [ ] **Step 1: 写失败的测试**

在 `tests/sudoku_test.cc` 追加：

```cpp
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
```

- [ ] **Step 2: 确认测试失败**

```bash
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset debug"
```

Expected: 链接失败，`generate` 未定义。

- [ ] **Step 3: 实现生成器**

在 `src/sudoku.cc` 中实现：

1. 内部常量：
   ```cpp
   struct ClueSpec
   {
       int target;
       int maxAllowed;
   };
   // Easy {40,46}, Medium {34,40}, Hard {30,36}, Master {26,32}
   ```
2. `makeSolution(std::mt19937&)`：
   - 基础模式 `base(r, c) = (r * 3 + r / 3 + c) % 9`。
   - 随机打乱数字 `1..9` 的置换 `digits`。
   - 随机打乱行带顺序与每个带内三行，得到 `rowOrder`；列用同样方式得到 `colOrder`。
   - `solution[r][c] = digits[base(rowOrder[r], colOrder[c])]`。
3. `generate(difficulty, rng)`：
   - 最多尝试 4 次。
   - 每次用 `makeSolution` 生成完整解，复制为 `givens`，将 81 个位置随机打乱。
   - 依次尝试清空位置，并用 `countSolutions(givens, 2) == 1` 判断唯一性；不唯一则恢复。
   - 线索数达到目标立即返回该次结果。
   - 若线索数 `<= maxAllowed` 也返回；否则记录线索最少的一次。
   - 4 次后返回记录的最佳结果。
4. `generate(difficulty)`：用 `std::random_device` 播种 `std::mt19937` 后调用上面的重载。

- [ ] **Step 4: 运行测试确认通过**

```bash
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset debug"
./Intermediate/Siv3D_Template/Tests/Debug/siv3d_template_tests.exe --gtest_filter='SudokuGenerator.*'
./Intermediate/Siv3D_Template/Tests/Debug/siv3d_template_tests.exe
```

Expected: 新增 3 个测试 PASS，且整个测试可执行文件全部 PASS。

---

### Task 4: 应用外壳与难度选择界面

**Files:**
- Modify: `src/Main.cpp`（整体替换模板示例）
- Read: `include/owl_sudoku/sudoku.h`

**Interfaces:**
- Consumes: `owl_sudoku::Difficulty`、`owl_sudoku::generate(Difficulty)`、`owl_sudoku::Puzzle`。
- Produces: 供 Task 5 使用的 `Screen` 枚举、`AppState` 结构、`startGame(AppState&, Difficulty)`、`drawButton(const Font&, const String&, const RectF&) -> bool`。

- [ ] **Step 1: 实现应用外壳、菜单与自定义按钮**

替换 `src/Main.cpp`：

```cpp
#include <Siv3D.hpp>
#include "owl_sudoku/sudoku.h"

namespace
{
	enum class Screen
	{
		Menu,
		Playing,
	};

	struct AppState
	{
		Screen screen = Screen::Menu;
		owl_sudoku::Difficulty difficulty = owl_sudoku::Difficulty::Easy;
		owl_sudoku::Puzzle puzzle{};
		owl_sudoku::Grid player{};
		Optional<Point> selected; // Siv3D Optional，支持 = none
		bool cleared = false;
	};

	bool drawButton(const Font& font, const String& label, const RectF& rect)
	{
		const bool hovered = rect.mouseOver();
		const ColorF fill = hovered ? ColorF{ 0.24, 0.30, 0.40 } : ColorF{ 0.17, 0.21, 0.29 };
		rect.draw(fill);
		rect.drawFrame(2.0, hovered ? ColorF{ 0.98, 0.72, 0.28 } : ColorF{ 0.45, 0.52, 0.62 });
		font(label).drawAt(rect.center(), ColorF{ 0.94, 0.96, 1.0 });
		return hovered && MouseL.down();
	}

	void startGame(AppState& state, owl_sudoku::Difficulty difficulty)
	{
		state.difficulty = difficulty;
		state.puzzle = owl_sudoku::generate(difficulty);
		state.player = state.puzzle.givens;
		state.selected = none;
		state.cleared = false;
		state.screen = Screen::Playing;
	}
}
```

`Main()` 中：

- `Window::SetTitle(U"Owl Sudoku");`
- `Window::Resize(960, 800);`
- `Scene::SetBackground(ColorF{ 0.10, 0.12, 0.16 });`
- 创建字体：
  ```cpp
  const Font titleFont{ 72, Typeface::Black };
  const Font headingFont{ 34, Typeface::CJK_Regular_SC };
  const Font uiFont{ 24, Typeface::CJK_Regular_SC };
  const Font smallFont{ 18, Typeface::CJK_Regular_SC };
  const Font numberFont{ 38, Typeface::Bold };
  ```
  另创建 `Font emojiFont{ 64, Typeface::ColorEmoji };` 并 `titleFont.addFallback(emojiFont);`。
- `while (System::Update())` 中按 `state.screen` 分支。

菜单界面：

- 标题 `🦉 Owl Sudoku`（`titleFont`，居中于 y≈160）。
- 副标题 `基础版`（`headingFont`，居中于 y≈235）。
- 提示 `选择难度`（`uiFont`，居中于 y≈290）。
- 四个按钮，`RectF{ 350, 320 + i * 84, 260, 64 }`，文字依次为 `简单`、`中等`、`困难`、`大师`。
- 点击任意按钮调用 `startGame(state, 对应 Difficulty)`。

占位游戏界面（Task 5 会替换为完整棋盘）：

- 顶部显示 `难度：简单/中等/困难/大师`。
- 中央显示 `棋盘将在下一步实现`。
- 提供 `RectF{ 800, 26, 120, 46 }` 的“新游戏”按钮，点击后 `state.screen = Screen::Menu;`。

难度到中文名的映射用局部辅助函数：

```cpp
StringView difficultyName(owl_sudoku::Difficulty difficulty)
{
	switch (difficulty)
	{
	case owl_sudoku::Difficulty::Easy:   return U"简单";
	case owl_sudoku::Difficulty::Medium: return U"中等";
	case owl_sudoku::Difficulty::Hard:   return U"困难";
	case owl_sudoku::Difficulty::Master: return U"大师";
	}
	return U"未知";
}
```

- [ ] **Step 2: 构建并手工验证菜单**

```bash
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset debug"
./App/Siv3D_Template\(debug\).exe
```

Expected:
- 窗口标题为 `Owl Sudoku`，尺寸 960×800。
- 四个难度按钮悬停变亮。
- 点击任一按钮进入占位游戏界面，并显示所选难度。
- 点击“新游戏”返回菜单。

---

### Task 5: 棋盘、输入、错误标记与通关

**Files:**
- Modify: `src/Main.cpp`

**Interfaces:**
- Consumes: Task 4 的 `AppState`、`drawButton`、`startGame`、`difficultyName`；核心模块的 `Grid`、`Puzzle`、`generate`。
- Produces: 完整可玩的游戏界面，无后续任务。

- [ ] **Step 1: 实现棋盘绘制**

在 `Main.cpp` 增加常量：

```cpp
constexpr double CellSize = 60.0;
constexpr Vec2 BoardOrigin{ 210.0, 130.0 };
```

绘制顺序：

1. 为每个格子计算 `RectF{ BoardOrigin.x + c * CellSize, BoardOrigin.y + r * CellSize, CellSize, CellSize }`。
2. 背景色：
   - 玩家填错（`player[r][c] != 0 && player[r][c] != puzzle.solution[r][c]`）：`ColorF{1.0, 0.88, 0.88}`
   - 选中格：`ColorF{0.80, 0.90, 1.0}`
   - 其他：`ColorF{0.97, 0.98, 1.0}`
3. 数字颜色：
   - 题目给定（`puzzle.givens[r][c] != 0`）：`ColorF{0.12, 0.15, 0.20}`
   - 玩家正确：`ColorF{0.12, 0.45, 0.85}`
   - 玩家错误：`ColorF{0.86, 0.20, 0.24}`
   - 用 `numberFont` 的 `drawAt(cellRect.center(), color)` 绘制。
4. 网格线：
   - 普通分隔线 1px，`ColorF{0.70, 0.74, 0.80}`。
   - 每 3 格及外框 3px，`ColorF{0.20, 0.24, 0.30}`。
   - 先画细线，再画粗线，避免被覆盖。
5. 底部提示：`1-9 填数 · Backspace 清除 · 方向键移动`，用小号中文字体居中绘制于 y≈710。

- [ ] **Step 2: 实现鼠标与键盘输入**

- 点击棋盘：由 `Cursor::PosF()` 计算行列；在 0..8 范围内则 `selected = Point{ r, c }`。
- 数字输入：
  ```cpp
  const std::array<Input, 9> topRow{ Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9 };
  const std::array<Input, 9> numpad{ KeyNum1, KeyNum2, KeyNum3, KeyNum4, KeyNum5,
                                     KeyNum6, KeyNum7, KeyNum8, KeyNum9 };
  ```
  任一按下且 `selected` 有值、`puzzle.givens[r][c] == 0` 时写入 `player[r][c] = digit`。
- `KeyBackspace.down() || KeyDelete.down()`：同样条件下清零。
- 方向键：移动选中格，边界处停止；没有选中格时从中心 `{4, 4}` 开始。
- 每次写入或清除后检查 `player == puzzle.solution`，成立则 `cleared = true`。
- 当 `cleared == true` 时跳过棋盘点击、键盘输入以及右上角“新游戏”按钮，只处理遮罩上的两个按钮；这保证遮罩期间的点击不会穿透到棋盘。

- [ ] **Step 3: 实现新游戏与通关遮罩**

- 游戏界面右上角“新游戏”按钮：`state.screen = Screen::Menu; state.selected = none; state.cleared = false;`。
- `cleared` 时绘制全屏 `RectF{ 0, 0, 960, 800 }`，颜色 `ColorF{0.0, 0.0, 0.0, 0.60}`。
- 中央面板 `RectF{ 280, 270, 400, 260 }`，颜色 `ColorF{0.13, 0.16, 0.22}`，边框琥珀色。
- 面板内容：
  - `完成！`（`headingFont`，居中于 y≈330）。
  - “再来一局”按钮 `RectF{ 330, 390, 300, 56 }`：重新调用 `startGame(state, state.difficulty)`。
  - “返回难度选择”按钮 `RectF{ 330, 462, 300, 56 }`：`state.screen = Screen::Menu; state.cleared = false;`。
- 遮罩绘制在棋盘之后；按钮点击检测也放在棋盘输入之后，确保遮罩优先。

- [ ] **Step 4: 构建并执行完整手工检查**

```bash
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset debug"
./App/Siv3D_Template\(debug\).exe
```

逐项确认：

1. 四个难度都能生成棋盘。
2. 点击空格后按 `1`–`9`（含小键盘）能填入；错误数字立即变红。
3. 点击题目给定的格子后输入，数字不变。
4. `Backspace` / `Delete` 能清除玩家填入的数字，不能清除给定数字。
5. 方向键能移动选中格。
6. 完成一局后出现遮罩；点击遮罩后的棋盘区域不会修改棋盘。
7. “再来一局”生成同难度新棋盘，且选区/遮罩状态已重置。
8. “返回难度选择”回到菜单；再次选择其他难度时状态干净。
9. 游戏中点“新游戏”返回菜单，再开局无旧棋盘残留。
10. 无选中格时按数字键、按 `0` 或其他键，程序无反应且不崩溃。

为便于验证通关流程，可在 Debug 构建中临时加入 `SIV3D_BUILD(DEBUG)` 守卫的 `F9` 快捷键，将 `player = puzzle.solution;`；验证完成后可保留（仅 Debug 生效）或删除。

- [ ] **Step 5: 全量测试与 Release 构建**

```bash
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset debug"
ctest --preset test-debug
cmd.exe /c "tools\\cmake-msvc.cmd --build --preset release"
ctest --preset test-release
```

Expected:
- 两个配置都编译通过。
- 两套 CTest 均为 100% 通过。
- `App/` 下生成可运行的 Debug 与 Release 可执行文件（Debug 文件名为 `Siv3D_Template(debug).exe`）。
