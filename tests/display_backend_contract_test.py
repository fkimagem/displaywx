"""Static backend contracts; no Arduino compilation or external library writes.

Run: python tests/display_backend_contract_test.py
Uses the sibling display libraries installed next to displaywx to check method
names and argument counts. This deliberately does not claim C++ type checking.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
LIBS = ROOT.parent
BACKENDS = ("DEFAULT", "PCD8544", "SSD1306", "U8G2", "ST7565")


def strip_comments(source):
    return re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*.*?\*/',
                  lambda m: "\n" * m[0].count("\n") if m[0].startswith("/") else m[0],
                  source, flags=re.S)


def condition(expression, macros):
    expression = re.sub(r'ESP_IDF_VERSION_VAL\((\d+),\s*(\d+),\s*(\d+)\)',
                        lambda m: str((int(m[1]) << 16) | (int(m[2]) << 8) | int(m[3])), expression)
    expression = re.sub(r'defined\s*\(\s*(\w+)\s*\)', lambda m: str(int(m[1] in macros)), expression)
    expression = re.sub(r'\b[A-Za-z_]\w*\b', lambda m: str(macros.get(m[0], "0") or "1"), expression)
    expression = expression.replace("&&", " and ").replace("||", " or ")
    expression = re.sub(r'!(?!=)', ' not ', expression)
    return bool(eval(expression.strip(), {"__builtins__": {}}, {}))


def active_source(path, macros):
    """Evaluate the conditional directives used by DisplayWX, preserving lines.

    Includes are not expanded: callers seed backend configuration explicitly.
    """
    source = strip_comments(path.read_text(encoding="utf-8-sig"))
    source = re.sub(r'\\\n', ' ', source)
    stack, output, enabled = [], [], True
    for number, line in enumerate(source.splitlines(), 1):
        directive = re.match(r'\s*#\s*(\w+)\s*(.*)', line)
        if directive:
            op, arg = directive.groups()
            if op in ("if", "ifdef", "ifndef"):
                match = (arg in macros if op == "ifdef" else arg not in macros if op == "ifndef"
                         else condition(arg, macros)) if enabled else False
                stack.append([enabled, match])
                enabled = enabled and match
            elif op == "elif":
                parent, taken = stack[-1]
                enabled = parent and not taken and condition(arg, macros)
                stack[-1][1] |= enabled
            elif op == "else":
                parent, taken = stack[-1]
                enabled = parent and not taken
                stack[-1][1] = True
            elif op == "endif":
                enabled = stack.pop()[0]
            elif enabled and op == "define":
                name, _, value = arg.partition(" ")
                if "(" not in name:
                    macros[name] = value.strip() or "1"
            elif enabled and op == "undef":
                macros.pop(arg.strip(), None)
            elif enabled and op == "error":
                raise AssertionError(f"{path}:{number}: active #error {arg}")
            output.append("")
        else:
            output.append(line if enabled else "")
    assert not stack, f"Unbalanced preprocessor directives: {path}"
    return "\n".join(output)


def arguments(source, opening):
    depth, start, items = 1, opening + 1, []
    token = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[(){}\[\],]')
    for match in token.finditer(source, start):
        char = match[0]
        if char in ("(", "{", "["):
            depth += 1
        elif char in (")", "}", "]"):
            depth -= 1
            if depth == 0:
                items.append(source[start:match.start()].strip())
                return [] if items == [""] or items == ["void"] else items
        elif char == "," and depth == 1:
            items.append(source[start:match.start()].strip())
            start = match.end()
    raise AssertionError("Unclosed argument list")


def api_signatures(backend):
    if backend == "DEFAULT":
        paths = [LIBS / "GFX_Library_for_Arduino/src/Arduino_GFX.h"]
    elif backend == "U8G2":
        paths = [LIBS / "U8g2_Arduino-master/src/U8g2lib.h"]
    else:
        paths = [LIBS / "Adafruit-GFX-Library-master/Adafruit_GFX.h"]
        paths.append(LIBS / {"SSD1306": "Adafruit_SSD1306-master/Adafruit_SSD1306.h",
                            "PCD8544": "Adafruit-PCD8544/Adafruit_PCD8544.h",
                            "ST7565": "ST7565_LCD-main/ST7565_LCD.h"}[backend])
    signatures = {"print": [(1, 2)]}  # Inherited from Arduino Print.
    for path in paths:
        source = strip_comments(path.read_text(encoding="utf-8-sig"))
        # Only the actual display/base class, not canvas or driver subclasses.
        if backend == "U8G2":
            source = source[:source.index("class U8G2LOG\n")]
        elif "Adafruit_GFX.h" == path.name:
            source = source[:source.index("class Adafruit_GFX_Button")]
        for match in re.finditer(r'^\s*(?:virtual\s+)?(?:void|bool|int\d*_t|uint\d*_t|size_t|u8g2_uint_t)\s+(\w+)\s*\(', source, re.M):
            params = arguments(source, match.end() - 1)
            signatures.setdefault(match[1], []).append((sum("=" not in p for p in params), len(params)))
    return signatures


def run():
    files = sorted((ROOT / "src/widgets").rglob("*.cpp")) + sorted((ROOT / "src/widgets").rglob("*.h"))
    files += [ROOT / name for name in ("src/displaywx.cpp", "src/displaywx.h", "src/touch/touch.cpp", "src/touch/touch.h")]
    cases = [({}, "normal"),
             ({"TOUCH_XPT2046": "1", "DEBUG_DISPLAY_FK": "1", "ENABLE_ON_RELEASE": "1"}, "touch/debug"),
             ({"TOUCH_FT6236U": "1"}, "capacitive touch"),
             ({"DFK_DISABLE_IMAGE": "1", "DFK_DISABLE_SD": "1"}, "no image/SD"),
             ({"CONFIG_IDF_TARGET_ESP32P4": "1", "CAM_OV5647": "1", "ESP_IDF_VERSION": str(0x050300)}, "old IDF/camera"),
             ({"CAM_OV5647": "1"}, "non-P4/camera")]
    cases += [({"CONFIG_IDF_TARGET_ESP32P4": "1", sensor: "1"}, sensor)
              for sensor in ("CAM_OV5647", "CAM_OV02C10", "CAM_SC2336", "CAM_OV2710")]
    total_calls = 0
    for backend in BACKENDS:
        api = api_signatures(backend)
        for extra, name in cases:
            macros = {f"DISP_{backend}": "1", "ESP_IDF_VERSION": str(0x050400), "ARDUINO": "10800", "CORE_DEBUG_LEVEL": "5"}
            macros.update(extra)
            for config in ("src/widgets/widgetsetup.h", "src/check_touch.h", "src/check_camera.h", "src/touch_widgets.h", "src/widgets/widgetbase.h", "src/displaywx.h"):
                active_source(ROOT / config, macros)
            if backend != "DEFAULT":
                assert "DFK_CAMERA_MIPI" not in macros and "DFK_CIRCULARBAR" not in macros
                assert "HAS_TOUCH" not in macros
            for capability in ("DFK_GAUGE", "DFK_LINECHART", "DFK_VANALOG", "DFK_LED"):
                assert (capability in macros) == (backend != "U8G2"), (backend, capability)
            assert ("DFK_IMAGE" in macros) == ("DFK_DISABLE_IMAGE" not in extra)
            assert ("DFK_SD" in macros) == ("DFK_DISABLE_SD" not in extra)
            if "HAS_CAMERA_MIPI" not in macros:
                for driver in (ROOT / "src/camera").rglob("*.c"):
                    assert not active_source(driver, dict(macros)).strip(), (backend, name, driver, "camera driver still enabled")
            for path in files:
                local = dict(macros)
                # Re-evaluate each header even if a seeded configuration included it.
                header_guard = re.search(r'^\s*#ifndef\s+(\w+)', strip_comments(path.read_text(encoding="utf-8-sig")), re.M)
                if header_guard and path.suffix == ".h":
                    local.pop(header_guard[1], None)
                code = active_source(path, local)
                delimiters = []
                tokens = re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', '', code)
                for char in tokens:
                    if char in "({[":
                        delimiters.append(char)
                    elif char in ")}]":
                        assert delimiters and delimiters.pop() == {')': '(', '}': '{', ']': '['}[char], (backend, path, "unbalanced delimiters")
                assert not delimiters, (backend, path, "unclosed delimiters")
                if backend != "DEFAULT":
                    assert not re.search(r'\bArduino_(?:GFX|DSI_Display)\b', code), (backend, path, "GFX-only type")
                if backend == "U8G2":
                    assert not re.search(r'\bGFXfont\b', code), (backend, path, "GFX font")
                for match in re.finditer(r'\b(?:objTFT|m_objTFT|tft)\s*->\s*(\w+)\s*\(', code):
                    method = match[1]
                    count = len(arguments(code, match.end() - 1))
                    line = code.count("\n", 0, match.start()) + 1
                    assert any(lo <= count <= hi for lo, hi in api.get(method, [])), (backend, name, str(path.relative_to(ROOT)), line, method, count)
                    total_calls += 1
                if path.name == "widgetbase.cpp":
                    for method in ("updateFont", "showOrigin", "isInitialized", "setEnabled", "lock", "unlock", "isLocked"):
                        assert f"WidgetBase::{method}(" in code, (backend, method, "missing definition")
                if path.name == "wcircularbar.cpp" and backend != "DEFAULT":
                    assert "drawArcWrap" not in code and "fillArc" not in code
            print(f"PASS DISP_{backend}: {name}")
    print(f"Checked {len(BACKENDS) * len(cases)} configurations, {total_calls} active display calls against local library headers.")


if __name__ == "__main__":
    run()
