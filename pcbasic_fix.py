"""Start PC-BASIC with fixes for two bugs (pcbasic 2.0.8; the first is also still in upstream master).

1. Temp-string leak.
   StringFunctions.left_/right_/mid_/instr_ register their string arguments in
   memory.temp_values, but return early (empty result, start past end, INSTR not found)
   without removing them again. Leaked temps pile up, string space fills, garbage
   collection runs constantly and eventually hits a freed string:
       KeyError: Dereferencing detached string at ...
   Fix: after each call, drop whatever the call added to temp_values and left behind.

2. Command-line interface (--interface=cli) does not scroll.
   VideoCLI only writes a newline to the terminal when the cursor moves to a lower row.
   Printing on the bottom row scrolls the virtual screen but keeps the cursor on row 25,
   so every new line is redrawn with '\r' over the same terminal line. It only catches
   up when the cursor becomes visible (LINE INPUT), and then only for one screenful.
   Fix: after a scroll-up, move the terminal down to the cursor row straight away.

Use: python pcbasic_fix.py [pcbasic options...]   (same arguments as pcbasic)
"""
import functools
import sys

from pcbasic.basic.values import values as _values
from pcbasic.interface import video_cli as _video_cli


def _cleanup_temps(method):
    @functools.wraps(method)
    def wrapper(self, args):
        before = set(self._temp_values)
        try:
            return method(self, args)
        finally:
            leaked = self._temp_values - before
            if leaked:
                self._temp_values.difference_update(leaked)
    return wrapper


for _name in ('left_', 'right_', 'mid_', 'instr_'):
    setattr(_values.StringFunctions, _name, _cleanup_temps(getattr(_values.StringFunctions, _name)))


_cli_scroll = _video_cli.VideoCLI.scroll


def _scroll_and_follow(self, direction, start_row, stop_row, back_attr):
    _cli_scroll(self, direction, start_row, stop_row, back_attr)
    if direction == -1 and self._last_row < self._cursor_row:
        self._update_position(self._cursor_row, self._cursor_col)


_video_cli.VideoCLI.scroll = _scroll_and_follow


if __name__ == '__main__':
    from pcbasic import main
    main(*sys.argv[1:])
