set pagination off
set confirm off
set breakpoint pending on
set debuginfod enabled off
set print thread-events off
set direct-call-timeout 10
set unwind-on-timeout on
python
import gdb, json, os, pathlib, struct, subprocess
if os.environ.get('TH08_BACKGROUND_SKIP_ENEMY_NAME') == '1':
    gdb.execute('break th08::Gui::CopyEnemyNameTexture(int)')
    gdb.execute('commands\nsilent\nreturn\ncontinue\nend')
out = pathlib.Path(os.environ['TH08_BACKGROUND_CAPTURE_DIR'])
stage = int(os.environ.get('TH08_BACKGROUND_STAGE', '5'))
frame = int(os.environ.get('TH08_BACKGROUND_FRAME', '6166'))
scene, draws = [], []
def value(expression): return gdb.parse_and_eval(expression)
def number(expression): return int(value(expression))
def block(expression, size):
    return bytes(gdb.selected_inferior().read_memory(int(value(expression).address), size))
def floats(expression, count): return list(struct.unpack('<%df' % count, block(expression, count*4)))
def active(): return number('th08::g_GameManager.currentStage') == stage and number('th08::g_GameManager.stageActiveFrames') == frame
class CameraExit(gdb.FinishBreakpoint):
    def stop(self):
        item = {'stage':stage, 'activeFrame':frame,
            'cameraCurrent':floats('th08::g_Background.cameraCurrent',19),
            'stagePosition':floats('th08::g_Background.stagePosition',3),
            'stageScriptTimerHex':block('th08::g_Background.stageScriptTimer',12).hex(),
            'cameraInterpolationTimersHex':block('th08::g_Background.cameraInterpolationTimers',60).hex(),
            'stageScriptInstructionIndex':number('th08::g_Background.stageScriptInstructionIndex'),
            'frameCounter':number('th08::g_Background.frameCounter'),
            'cameraMotionMode':number('th08::g_Background.cameraMotionMode'),
            'view':floats('th08::g_Supervisor.viewMatrix',16),
            'projection':floats('th08::g_Supervisor.projectionMatrix',16),
            'viewport':[number('th08::g_Supervisor.viewport.'+field) for field in ('X','Y','Width','Height')]}
        scene.append(item)
        (out/'camera.json').write_text(json.dumps(scene,indent=2)+chr(10))
        draw.enabled = True
        return False
class CameraEntry(gdb.Breakpoint):
    def __init__(self):
        super().__init__('th08::Background::SetCamera2()',internal=True)
        self.enabled = False
    def stop(self):
        if active(): CameraExit(internal=True)
        return False
camera = CameraEntry()
class Draw(gdb.Breakpoint):
    def __init__(self):
        super().__init__("'(anonymous namespace)::LinuxDevice::Draw'",internal=True)
        self.enabled = False
    def stop(self):
        if not active() or len(draws) >= 5: return False
        fvf = number('this->fvf')
        if fvf & 14 != 2: return False
        stride, primitive_count, primitive_type = number('stride'), number('primitiveCount'), number('type')
        count = {1:primitive_count,2:primitive_count*2,3:primitive_count+1,4:primitive_count*3,5:primitive_count+2,6:primitive_count+2}[primitive_type]
        raw = bytes(gdb.selected_inferior().read_memory(number('data'),min(count,12)*stride))
        vertices=[]
        for i in range(min(count,12)):
            vertices.append({'position':list(struct.unpack_from('<3f',raw,i*stride)),
                'uv':list(struct.unpack_from('<2f',raw,i*stride+16)) if fvf == 0x142 else list(struct.unpack_from('<2f',raw,i*stride+12)) if fvf == 0x102 else None})
        draws.append({'fvf':fvf,'stride':stride,'primitiveType':primitive_type,
            'primitiveCount':primitive_count,'vertices':vertices,
            'world':floats('this->world',16),'view':floats('this->view',16),
            'projection':floats('this->projection',16)})
        (out/'draws.json').write_text(json.dumps(draws,indent=2)+chr(10))
        if len(draws) == 5: self.enabled = False
        return False
draw = Draw()
class Present(gdb.Breakpoint):
    def __init__(self):
        super().__init__("'(anonymous namespace)::LinuxDevice::Present'",internal=True)
        self.enabled = False
    def stop(self):
        if active():
            gdb.execute('set $background_device = this')
            Presented(internal=True)
        return False
class Presented(gdb.FinishBreakpoint):
    def stop(self): return True
present = Present()
class Arm(gdb.Breakpoint):
    def __init__(self): super().__init__('th08::GameWindow::Present()',internal=True)
    def stop(self):
        if number('th08::g_GameManager.currentStage') == stage and number('th08::g_GameManager.stageActiveFrames') >= frame - 1:
            self.enabled = False
            camera.enabled = present.enabled = True
        return False
arm = Arm()
def screenshot():
    if not active() or not scene: raise RuntimeError('Requested frame or camera evidence is missing.')
    gdb.execute('set scheduler-locking off')
    gdb.execute('set $background_access = (LinuxSurfaceAccess*)malloc(sizeof(LinuxSurfaceAccess))')
    gdb.execute('call th08_linux_surface_access($background_device->backbuffer, $background_access, true)')
    access = value('*$background_access')
    width,height,pitch=(int(access[k]) for k in ('width','height','pitch'))
    if int(access['format']) != 22 or (width,height,pitch) != (640,480,2560): raise RuntimeError('Unexpected framebuffer format.')
    pixels=bytes(gdb.selected_inferior().read_memory(int(access['pixels']),pitch*height))
    gdb.execute('call free($background_access)')
    converter='from PIL import Image; import sys; Image.frombytes("RGB",(640,480),sys.stdin.buffer.read(),"raw","BGRX",2560,1).save(sys.argv[1])'
    subprocess.run(['python3','-c',converter,str(out/f'stage{stage}-frame-{frame}.png')],input=pixels,check=True,timeout=15)
    print(f'Captured stage index {stage}, frame {frame}, with camera, matrices, and 3D draw evidence.')
end
run
python screenshot()
kill
quit
