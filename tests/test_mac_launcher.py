"""Check save/config routing and input validation without starting Wine or a game."""
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import Mock

spec = importlib.util.spec_from_file_location('play_mac', Path(__file__).resolve().parents[1] / 'tools/play-mac.py')
launcher = importlib.util.module_from_spec(spec)
spec.loader.exec_module(launcher)


class MacLauncherTests(unittest.TestCase):
    def test_paths_with_spaces_and_stale_test_environment(self):
        env = launcher.game_environment({
            'PATH': '/usr/bin', 'RECOMP_SAVE_DIR': 'old-test-saves',
            'RECOMP_INPUT_SEQ': 'auto-fire', 'TSFP_PC_INPUT_REPLAY': 'old-replay',
            'RECOMP_HLE_D3D8_DUMP': 'old-capture', 'TSFP_TEXTURE_PACK': 'old-pack',
        }, r'Z:\Users\Example User\Time Splitters', 'test', (0.1, True, False))
        self.assertEqual(env['RECOMP_SAVE_DIR'], r'Z:\Users\Example User\Time Splitters\private\saves')
        self.assertEqual(env['TSFP_DISPLAY_CONFIG'], r'Z:\Users\Example User\Time Splitters\config\pc-display.ini')
        self.assertEqual(env['RECOMP_INPUT_CONFIG'], r'Z:\Users\Example User\Time Splitters\config\pc-controls.json')
        self.assertEqual(env['PATH'], '/usr/bin')
        for key in ('RECOMP_INPUT_SEQ', 'TSFP_PC_INPUT_REPLAY', 'RECOMP_HLE_D3D8_DUMP', 'TSFP_TEXTURE_PACK'):
            self.assertNotIn(key, env)

    def test_invalid_mouse_settings_stop_before_launch(self):
        for value in ('NaN', 'Infinity', '-1', '0', '10.1', 'true', '"0.1"'):
            with self.subTest(value=value):
                path = Mock()
                path.read_text.return_value = '{"mouse":{"sensitivity":' + value + '}}'
                with self.assertRaises(ValueError):
                    launcher.read_controls(path)

    def test_existing_fixture_is_supported(self):
        self.assertEqual(launcher.read_controls(launcher.ROOT / 'tests/fixtures/pc-controls.json'), (0.1, True, False))


if __name__ == '__main__':
    unittest.main()
