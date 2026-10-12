"""Use the repository capture runner with the local screenshot observer."""
import os
import pathlib
import sys
root = pathlib.Path(os.environ['TH08_STAGE5_REPOSITORY'])
sys.path.insert(0, str(root / 'scripts'))
script = root / 'scripts/capture-replay.py'
source = script.read_text()
needle = 'observer = Path(__file__).with_name("replay-external-observer.py" if replay else "replay-observer.py")'
replacement = 'observer = Path(os.environ["TH08_STAGE5_NATIVE_OBSERVER"])'
assert source.count(needle) == 1
source = source.replace(needle, replacement)
exec(compile(source, str(script), 'exec'), {'__file__': str(script), '__name__': '__main__'})
