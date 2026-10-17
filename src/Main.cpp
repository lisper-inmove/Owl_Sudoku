# include <Siv3D.hpp> // Siv3D v0.6.16

void Main()
{
	// Set the background color
	Scene::SetBackground(ColorF{ 0.6, 0.8, 0.7 });

	// Create a texture from an image file
	const Texture texture{ U"example/windmill.png" };

	// Create a texture from an emoji
	const Texture emoji{ U"🦖"_emoji };

	// Create a bold font with MSDF method
	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };

	// Create a font for emojis in text and add it to font as a fallback
	const Font emojiFont{ 48, Typeface::ColorEmoji };
	font.addFallback(emojiFont);

	// Number of button presses
	int32 count = 0;

	// Checkbox state
	bool checked = false;

	// Player's movement speed
	double speed = 200.0;

	// Player's X position
	double playerPosX = 400;

	// Whether player is facing right
	bool isPlayerFacingRight = true;

	while (System::Update())
	{
		// Draw the texture
		texture.draw(20, 20);

		// Draw text
		font(U"Hello, Siv3D!🎮").draw(64, Vec2{ 20, 340 }, ColorF{ 0.2, 0.4, 0.8 });

		// Draw text within a specified area
		font(U"Siv3D (シブスリーディー) は、ゲームやアプリを楽しく簡単な C++ コードで開発できるフレームワークです。")
			.draw(18, Rect{ 20, 430, 480, 200 }, Palette::Black);

		// Draw a rectangle
		Rect{ 540, 20, 80, 80 }.draw();

		// Draw a rounded rectangle
		RoundRect{ 680, 20, 80, 200, 20 }.draw(ColorF{ 0.0, 0.4, 0.6 });

		// Draw a circle
		Circle{ 580, 180, 40 }.draw(Palette::Seagreen);

		// Draw an arrow
		Line{ 540, 330, 760, 260 }.drawArrow(8, SizeF{ 20, 20 }, ColorF{ 0.4 });

		// Draw a semi-transparent circle
		Circle{ Cursor::Pos(), 40 }.draw(ColorF{ 1.0, 0.0, 0.0, 0.5 });

		// Button
		if (SimpleGUI::Button(U"count: {}"_fmt(count), Vec2{ 520, 370 }, 120, (checked == false)))
		{
			// Increase the count
			++count;
		}

		// Checkbox
		SimpleGUI::CheckBox(checked, U"Lock \U000F033E", Vec2{ 660, 370 }, 120);

		// Slider
		SimpleGUI::Slider(U"speed: {:.1f}"_fmt(speed), speed, 100, 400, Vec2{ 520, 420 }, 140, 120);

		// If left key is pressed
		if (KeyLeft.pressed())
		{
			// Player moves left
			playerPosX = Max((playerPosX - speed * Scene::DeltaTime()), 60.0);
			isPlayerFacingRight = false;
		}

		// If right key is pressed
		if (KeyRight.pressed())
		{
			// Player moves right
			playerPosX = Min((playerPosX + speed * Scene::DeltaTime()), 740.0);
			isPlayerFacingRight = true;
		}

		// Draw the player
		emoji.scaled(0.75).mirrored(isPlayerFacingRight).drawAt(playerPosX, 540);
	}
}

//
// - Debug build: less optimization, but detailed information is available on errors and crashes.
//
// - Release build: builds with maximum optimization.
//
// - Running the program via [Debug] > [Start Debugging] shows detailed logs in the [Output] window, which helps you find the cause of errors.
//
// - After updating Visual Studio, you may need to rebuild the program ([Build] > [Rebuild Solution]).
//
