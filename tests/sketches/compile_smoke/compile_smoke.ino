#include <displaywx.h>
#include <type_traits>

static_assert(!std::is_copy_constructible<DisplayWX>::value, "DisplayWX owns resources");
static_assert(!std::is_copy_constructible<RadioGroup>::value, "RadioGroup owns its buttons");
static_assert(!std::is_copy_constructible<Image>::value, "Image owns its file buffers");
static_assert(!std::is_copy_constructible<LineChart>::value, "LineChart owns its pool");

// Compile/link smoke test. Use the project's real sketch for hardware validation.
DisplayWX display;
void setup() {
    display.setup();
    display.setup();
    if (display.startCustomDraw(0)) display.finishCustomDraw();
    display.setupAutoClick(100, 10, 10);
    display.startAutoClick();
    display.stopAutoClick();
    display.createTask(false);
    display.end(2000);
}
void loop() { delay(100); }
