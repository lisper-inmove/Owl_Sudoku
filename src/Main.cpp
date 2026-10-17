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
		Optional<owl_sudoku::Cell> selected; // Siv3D Optional，支持 = none
		bool cleared = false;
		bool ignoreBoardClick = false;
	};

	constexpr double CellSize = 60.0;
	constexpr Vec2 BoardOrigin{ 210.0, 130.0 };
	constexpr double BoardPixels = CellSize * owl_sudoku::BoardSize;

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
		state.ignoreBoardClick = true;
		state.screen = Screen::Playing;
	}

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

	RectF cellRect(int row, int column)
	{
		return RectF{
			BoardOrigin.x + column * CellSize,
			BoardOrigin.y + row * CellSize,
			CellSize,
			CellSize,
		};
	}

	void drawBoard(const AppState& state, const Font& numberFont)
	{
		for (int row = 0; row < owl_sudoku::BoardSize; ++row)
		{
			for (int column = 0; column < owl_sudoku::BoardSize; ++column)
			{
				const RectF rect = cellRect(row, column);
				const int value = state.player[row][column];
				const bool editable = (state.puzzle.givens[row][column] == 0);
				const bool wrong = editable && (value != 0)
					&& owl_sudoku::hasConflict(state.player, row, column);
				const bool selected = state.selected
					&& (state.selected->row == row) && (state.selected->column == column);

				ColorF background = ColorF{ 0.97, 0.98, 1.0 };

				if (wrong)
				{
					background = ColorF{ 1.0, 0.88, 0.88 };
				}

				if (selected)
				{
					background = ColorF{ 0.80, 0.90, 1.0 };
				}

				rect.draw(background);

				if (value != 0)
				{
					ColorF textColor = ColorF{ 0.12, 0.45, 0.85 };

					if (state.puzzle.givens[row][column] != 0)
					{
						textColor = ColorF{ 0.12, 0.15, 0.20 };
					}
					else if (wrong)
					{
						textColor = ColorF{ 0.86, 0.20, 0.24 };
					}

					numberFont(U"{}"_fmt(value)).drawAt(rect.center(), textColor);
				}
			}
		}

		for (int i = 0; i <= owl_sudoku::BoardSize; ++i)
		{
			const bool thick = (i % owl_sudoku::BoxSize == 0);
			const double thickness = thick ? 3.0 : 1.0;
			const ColorF color = thick ? ColorF{ 0.20, 0.24, 0.30 } : ColorF{ 0.70, 0.74, 0.80 };
			const double x = BoardOrigin.x + i * CellSize;
			const double y = BoardOrigin.y + i * CellSize;

			Line{ x, BoardOrigin.y, x, BoardOrigin.y + BoardPixels }.draw(thickness, color);
			Line{ BoardOrigin.x, y, BoardOrigin.x + BoardPixels, y }.draw(thickness, color);
		}

		if (state.selected)
		{
			cellRect(state.selected->row, state.selected->column)
				.drawFrame(3.0, ColorF{ 0.98, 0.72, 0.28 });
		}
	}

	void handleBoardInput(AppState& state)
	{
#if SIV3D_BUILD(DEBUG)
		if (KeyF9.down())
		{
			state.player = state.puzzle.solution;
			state.cleared = true;
			return;
		}
#endif

		if (state.ignoreBoardClick)
		{
			if (!MouseL.pressed() && !MouseL.down())
			{
				state.ignoreBoardClick = false;
			}
		}
		else if (MouseL.down())
		{
			const Vec2 cursor = Cursor::PosF();
			const double localX = cursor.x - BoardOrigin.x;
			const double localY = cursor.y - BoardOrigin.y;

			if (const auto cell = owl_sudoku::cellAt(localX, localY, CellSize))
			{
				state.selected = *cell;
			}
		}

		const bool arrowPressed = KeyLeft.down() || KeyRight.down() || KeyUp.down() || KeyDown.down();

		if (!state.selected)
		{
			if (arrowPressed)
			{
				state.selected = owl_sudoku::Cell{ 4, 4 };
			}
			else
			{
				return;
			}
		}

		int rowDelta = 0;
		int columnDelta = 0;

		if (KeyLeft.down())
		{
			columnDelta = -1;
		}
		else if (KeyRight.down())
		{
			columnDelta = 1;
		}
		else if (KeyUp.down())
		{
			rowDelta = -1;
		}
		else if (KeyDown.down())
		{
			rowDelta = 1;
		}

		const owl_sudoku::Cell cell = owl_sudoku::moveCell(*state.selected, rowDelta, columnDelta);
		state.selected = cell;

		if (state.puzzle.givens[cell.row][cell.column] != 0)
		{
			return;
		}

		const std::array<Input, 9> topRow{
			Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,
		};
		const std::array<Input, 9> numpad{
			KeyNum1, KeyNum2, KeyNum3, KeyNum4, KeyNum5,
			KeyNum6, KeyNum7, KeyNum8, KeyNum9,
		};

		int digit = 0;

		for (int i = 0; i < 9; ++i)
		{
			if (topRow[i].down() || numpad[i].down())
			{
				digit = i + 1;
				break;
			}
		}

		bool changed = false;

		if (digit != 0)
		{
			state.player[cell.row][cell.column] = digit;
			changed = true;
		}
		else if (KeyBackspace.down() || KeyDelete.down())
		{
			state.player[cell.row][cell.column] = 0;
			changed = true;
		}

		if (changed && (state.player == state.puzzle.solution))
		{
			state.cleared = true;
		}
	}

	void drawGame(AppState& state, const Font& headingFont, const Font& uiFont,
		const Font& smallFont, const Font& numberFont)
	{
		uiFont(U"难度：{}"_fmt(difficultyName(state.difficulty))).draw(40, 34, ColorF{ 0.78, 0.82, 0.90 });
		drawBoard(state, numberFont);
		smallFont(U"1-9 填数 · Backspace 清除 · 方向键移动").drawAt(Scene::Center().x, 710, ColorF{ 0.55, 0.60, 0.70 });

		if (!state.cleared)
		{
			if (drawButton(uiFont, U"新游戏", RectF{ 800, 26, 120, 46 }))
			{
				state.screen = Screen::Menu;
				state.selected = none;
				state.cleared = false;
				return;
			}

			handleBoardInput(state);
			return;
		}

		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.60 });

		const RectF panel{ 280, 270, 400, 260 };
		panel.draw(ColorF{ 0.13, 0.16, 0.22 });
		panel.drawFrame(2.0, ColorF{ 0.98, 0.72, 0.28 });
		headingFont(U"完成！").drawAt(Scene::Center().x, 330, ColorF{ 0.98, 0.72, 0.28 });

		if (drawButton(uiFont, U"再来一局", RectF{ 330, 390, 300, 56 }))
		{
			startGame(state, state.difficulty);
			return;
		}

		if (drawButton(uiFont, U"返回难度选择", RectF{ 330, 462, 300, 56 }))
		{
			state.screen = Screen::Menu;
			state.selected = none;
			state.cleared = false;
		}
	}

	void drawMenu(AppState& state, const Font& titleFont, const Font& headingFont,
		const Font& uiFont, const Font& smallFont)
	{
		const double centerX = Scene::Center().x;

		titleFont(U"🦉 Owl Sudoku").drawAt(centerX, 150, ColorF{ 0.94, 0.96, 1.0 });
		headingFont(U"基础版").drawAt(centerX, 225, ColorF{ 0.98, 0.72, 0.28 });
		uiFont(U"选择难度").drawAt(centerX, 285, ColorF{ 0.78, 0.82, 0.90 });
		smallFont(U"鼠标点击按钮开始游戏").drawAt(centerX, 700, ColorF{ 0.55, 0.60, 0.70 });

		const std::array<StringView, 4> labels{ U"简单", U"中等", U"困难", U"大师" };
		const std::array<owl_sudoku::Difficulty, 4> difficulties{
			owl_sudoku::Difficulty::Easy,
			owl_sudoku::Difficulty::Medium,
			owl_sudoku::Difficulty::Hard,
			owl_sudoku::Difficulty::Master,
		};

		for (int i = 0; i < 4; ++i)
		{
			const RectF rect{ 350, 320 + i * 84, 260, 64 };

			if (drawButton(uiFont, String{ labels[i] }, rect))
			{
				startGame(state, difficulties[i]);
				return;
			}
		}
	}
}

void Main()
{
	Window::SetTitle(U"Owl Sudoku");
	Window::Resize(960, 800);
	Scene::SetBackground(ColorF{ 0.10, 0.12, 0.16 });

	const Font titleFont{ 72, Typeface::Black };
	const Font headingFont{ 34, Typeface::CJK_Regular_SC };
	const Font uiFont{ 24, Typeface::CJK_Regular_SC };
	const Font smallFont{ 18, Typeface::CJK_Regular_SC };
	const Font numberFont{ 38, Typeface::Bold };
	const Font emojiFont{ 64, Typeface::ColorEmoji };
	titleFont.addFallback(emojiFont);

	AppState state;

	while (System::Update())
	{
		switch (state.screen)
		{
		case Screen::Menu:
			drawMenu(state, titleFont, headingFont, uiFont, smallFont);
			break;

		case Screen::Playing:
			drawGame(state, headingFont, uiFont, smallFont, numberFont);
			break;
		}
	}
}
