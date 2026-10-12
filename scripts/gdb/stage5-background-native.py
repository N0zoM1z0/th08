"""Read-only native replay observer with an attested post-Present screenshot."""
import gdb
import json
import os
import pathlib
import subprocess

root = pathlib.Path(os.environ['TH08_STAGE5_REPOSITORY'])
original = root / 'scripts/replay-external-observer.py'
ns = {'__file__': str(original), '__name__': 'stage5_replay_observer'}
exec(compile(original.read_text(), str(original), 'exec'), ns)
output = ns['output']
settings = ns['settings']
observer = ns['observer']
read = ns['read']
gm = ns['GM']
capture_settings = json.loads((output.parent / 'native-capture-settings.json').read_text())[output.name]
address = capture_settings['postPresentAddress']
expected = bytes.fromhex(capture_settings['postPresentBytes'])
if ns['block'](address, len(expected)) != expected:
    raise RuntimeError('Post-Present instructions do not match the attested executable.')

class Screenshot(gdb.Breakpoint):
    def __init__(self):
        super().__init__('*0x%08X' % address, gdb.BP_HARDWARE_BREAKPOINT)
        self.enabled = False
    def stop(self):
        stage, frame = read(gm + 0x3ddc4), read(gm + 0x3ddc0)
        if stage != 5 or frame < 6166:
            return False
        if frame != 6166:
            (output / 'complete.json').write_text(json.dumps({'rows': observer.count,
                'ended': False, 'diagnosticComplete': False,
                'errors': ['Presentation skipped requested active frame 6166; observed %d.' % frame]}) + chr(10))
            self.enabled = False
            return True
        pid = gdb.selected_inferior().pid
        windows = subprocess.check_output(['xdotool', 'search', '--onlyvisible', '--pid', str(pid)], text=True).split()
        if len(windows) != 1:
            raise RuntimeError('Expected one visible game window: ' + str(windows))
        image = output / 'stage5-frame-6166.png'
        geometry = subprocess.check_output(['xwininfo', '-id', windows[0]], text=False).decode('ascii', errors='replace')
        (output / 'window-geometry.txt').write_text(geometry)
        subprocess.run(['import', '-silent', '-window', windows[0], str(image)], check=True, timeout=15)
        snapshot = dict(stage=stage, activeFrame=frame,
            replayFrame=read(read(settings['replayManager'])),
            lastCalculation=observer.count, postPresentAddress=address,
            postPresentBytes=expected.hex(), captureMethod='X window readback after D3D8 Present returned',
            gameplayMemoryReadOnly=True, executableBytesUnmodified=True)
        (output / 'screenshot.json').write_text(json.dumps(snapshot, indent=2) + chr(10))
        for name in ('stream', 'menu_log', 'events', 'releases', 'score_events', 'item_events'):
            ns[name].flush()
            ns[name].close()
        for bp in (observer, observer.completion_observer, ns['begin_observer'], ns['delete_observer'], self):
            bp.enabled = False
        (output / 'complete.json').write_text(json.dumps({'rows': observer.count,
            'ended': False, 'diagnosticComplete': True, 'errors': [],
            'stage': stage, 'activeFrame': frame, 'screenshot': image.name}) + chr(10))
        print('Captured %s Stage 5 active frame 6166.' % output.name)
        return False

import struct
camera_exit = capture_settings['cameraExitAddress']
camera_expected = bytes.fromhex(capture_settings['cameraExitBytes'])
if ns['block'](camera_exit,len(camera_expected)) != camera_expected:
    raise RuntimeError('Camera exit instructions differ from the attested image.')
class Camera(gdb.Breakpoint):
    def __init__(self):
        super().__init__('*0x%08X' % camera_exit,gdb.BP_HARDWARE_BREAKPOINT)
        self.enabled=False
    def stop(self):
        if read(gm+0x3ddc4)!=5 or read(gm+0x3ddc0)!=6166:return False
        bg=capture_settings['background']
        sup=capture_settings['supervisor']
        floats=lambda addr,count:list(struct.unpack('<%df'%count,ns['block'](addr,count*4)))
        item={'stage':5,'activeFrame':6166,'cameraCurrent':floats(bg+0x6394,19),'stagePosition':floats(bg+0x824,3),
            'stageScriptTimerHex':ns['block'](bg+0x80c,12).hex(),'cameraInterpolationTimersHex':ns['block'](bg+0x63f4,60).hex(),
            'stageScriptInstructionIndex':read(bg+0x818),'frameCounter':read(bg+0x81c),'cameraMotionMode':ns['block'](bg+0x6474,1)[0],
            'view':floats(sup+0x48,16),'projection':floats(sup+0x88,16),
            'viewport':list(struct.unpack('<4I',ns['block'](sup+0xc8,16)))}
        path=output/'camera.json'
        items=json.loads(path.read_text()) if path.exists() else []
        items.append(item);path.write_text(json.dumps(items,indent=2)+chr(10))
        self.enabled=False
        print('Captured retail background camera and matrices at frame 6166.')
        return False
camera=Camera()
screenshot = Screenshot()
ordinary_stop = observer.stop
def observe_and_arm(completion=False):
    result = ordinary_stop(completion=completion)
    if observer.stage == 5 and read(gm + 0x3ddc0) >= 6150:
        screenshot.enabled = camera.enabled = True
    return result
observer.stop = observe_and_arm
