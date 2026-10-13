set pagination off
set confirm off
set debuginfod enabled off
set breakpoint pending on
set print thread-events off
set direct-call-timeout 10
set unwind-on-timeout on
python
import gdb, json, os, pathlib, subprocess, struct, hashlib
out=pathlib.Path(os.environ['TH08_DISPLAY_OUTPUT'])
stage=int(os.environ.get('TH08_DISPLAY_STAGE','0'))
frame=int(os.environ.get('TH08_DISPLAY_FRAME','600'))
def value(expr): return gdb.parse_and_eval(expr)
def number(expr): return int(value(expr))
class Present(gdb.Breakpoint):
    def __init__(self): super().__init__("'(anonymous namespace)::LinuxDevice::Present'",internal=True)
    def stop(self):
        if number('th08::g_GameManager.currentStage')==stage and number('th08::g_GameManager.stageActiveFrames')>=frame:
            gdb.execute('set $display_device = this')
            Presented(internal=True)
        return False
class Presented(gdb.FinishBreakpoint):
    def stop(self): return True
present=Present()
def call(expr): gdb.execute('call '+expr,to_string=True)
def capture(name):
    gdb.execute('set scheduler-locking off')
    gdb.execute('set $display_scratch = (char*)malloc(640*480*4)')
    call('(void)glReadPixels(0,0,640,480,6408,5121,$display_scratch)')
    raw=bytes(gdb.selected_inferior().read_memory(number('$display_scratch'),640*480*4))
    # Convert in the host process; GDB's embedded Python need not provide Pillow.
    converter='from PIL import Image; import sys; Image.frombytes("RGBA",(640,480),sys.stdin.buffer.read()).transpose(Image.Transpose.FLIP_TOP_BOTTOM).convert("RGB").save(sys.argv[1])'
    subprocess.run(['python3','-c',converter,str(out/(name+'-framebuffer.png'))],input=raw,check=True)
    start=number('$display_device->backbuffer->pixels._M_impl._M_start')
    cachebytes=bytes(gdb.selected_inferior().read_memory(start,640*480*4))
    converter='from PIL import Image; import sys; Image.frombytes("RGB",(640,480),sys.stdin.buffer.read(),"raw","BGRX",2560,1).save(sys.argv[1])'
    subprocess.run(['python3','-c',converter,str(out/(name+'-cpu-cache.png'))],input=cachebytes,check=True)
    item={'stage':number('th08::g_GameManager.currentStage'),'activeFrame':number('th08::g_GameManager.stageActiveFrames'),'backbufferDirty':bool(value('$display_device->backbuffer->dirty')),'framebufferSha256':hashlib.sha256(raw).hexdigest(),'cpuCacheSha256':hashlib.sha256(cachebytes).hexdigest()}
    call('(void)SDL_GetWindowSize($display_device->window,(int*)$display_scratch,(int*)$display_scratch+1)')
    item['sdlWindowSize']=list(struct.unpack('<2i',bytes(gdb.selected_inferior().read_memory(number('$display_scratch'),8))))
    call('(void)SDL_GL_GetDrawableSize($display_device->window,(int*)$display_scratch,(int*)$display_scratch+1)')
    item['sdlDrawableSize']=list(struct.unpack('<2i',bytes(gdb.selected_inferior().read_memory(number('$display_scratch'),8))))
    for label,param,count in [('viewport',2978,4),('scissorBox',3088,4),('framebufferBinding',36006,1)]:
        call('(void)glGetIntegerv(%d,(int*)$display_scratch)'%param)
        item[label]=list(struct.unpack('<%di'%count,bytes(gdb.selected_inferior().read_memory(number('$display_scratch'),count*4))))
    for label,param in [('textureMatrix',2984),('projectionMatrix',2983),('modelviewMatrix',2982)]:
        call('(void)glGetFloatv(%d,(float*)$display_scratch)'%param)
        item[label]=list(struct.unpack('<16f',bytes(gdb.selected_inferior().read_memory(number('$display_scratch'),64))))
    call('(void)free($display_scratch)')
    windows=subprocess.check_output(['xdotool','search','--onlyvisible','--pid',str(gdb.selected_inferior().pid)],text=True).split()
    if len(windows)!=1: raise RuntimeError('Expected one visible window.')
    item['windowId']=windows[0]
    (out/(name+'-window-geometry.txt')).write_bytes(subprocess.check_output(['xwininfo','-id',windows[0]]))
    subprocess.run(['import','-silent','-window',windows[0],str(out/(name+'-window.png'))],check=True)
    (out/(name+'.json')).write_text(json.dumps(item,indent=2)+chr(10))
    print('Captured '+name+' at active frame '+str(item['activeFrame']))
    return windows[0]
end
run
python
window=capture('before-home')
subprocess.run(['xdotool','windowfocus',window,'keydown','Home'],check=True)
end
continue
python
capture('after-home')
subprocess.run(['xdotool','keyup','Home'],check=True)
end
continue
python capture('next-frame')
kill
quit
