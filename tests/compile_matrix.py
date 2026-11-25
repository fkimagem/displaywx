from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
import subprocess, json, sys

import argparse, os
parser=argparse.ArgumentParser(description='Compile DisplayWX sources with the locally installed Arduino ESP32 3.3.12 toolchain (no upload).')
parser.add_argument('--packages',type=Path,default=Path(os.environ.get('ARDUINO_PACKAGES',r'C:/arduino_edgewx/portable/packages')))
parser.add_argument('--target',choices=['esp32','esp32s3','esp32p4'],default='esp32s3')
parser.add_argument('--backend',choices=['DEFAULT','PCD8544','SSD1306','ST7565','U8G2'],default='DEFAULT')
parser.add_argument('--touch',default='FT6236U')
parser.add_argument('--camera',default='')
parser.add_argument('--minimal',action='store_true')
parser.add_argument('--full-fonts',action='store_true')
parser.add_argument('--filter',default='')
parser.add_argument('--calibration',choices=['4','9'],default='9')
options=parser.parse_args()
root=Path(__file__).resolve().parents[1]
name='-'.join([options.target,options.backend,options.touch,options.camera or 'no-camera','minimal' if options.minimal else 'widgets','full' if options.full_fonts else 'compact','cal'+options.calibration])
out=root/'tmp/correcoes-build'/name
out.mkdir(parents=True,exist_ok=True)
platform=options.packages/'esp32/hardware/esp32/3.3.12'
sdk=options.packages/('esp32/tools/'+options.target+'-libs/3.3.12')
compiler=options.packages/('esp32/tools/esp-rv32/2601/bin/riscv32-esp-elf-g++.exe' if options.target=='esp32p4' else 'esp32/tools/esp-x32/2601/bin/xtensa-'+options.target+'-elf-g++.exe')
config=['DISP_'+options.backend,'TOUCH_'+options.touch]
config.append('CALIBRATE_POINTS '+options.calibration)
if options.camera: config.append('CAM_'+options.camera)
if not options.full_fonts: config.append('DFK_COMPACT_FONTS')
if options.minimal:
    config += ['DFK_DISABLE_'+x for x in ['SD','IMAGE','LOGGING','SPIFFS','FATFS','TEXTBOX','NUMBERBOX','CAMERA_MIPI','LINECHART','GAUGE','VBAR','VANALOG','THERMOMETER','CIRCULARBAR','SPINBOX','RADIO','HSLIDER','TEXTBUTTON','CIRCLEBTN','RECTBTN','CHECKBOX','TOGGLE','TOUCHAREA','LED']]
configfile=out/'config.h'
configfile.write_text(''.join('#define '+c+('' if c.startswith('CALIBRATE_POINTS ') else ' 1')+'\n' for c in config),encoding='utf-8')
memtype={'esp32':'qio_qspi','esp32s3':'qio_opi','esp32p4':'qio_qspi'}[options.target]
if not (sdk/memtype/'include').exists(): memtype='qio_qspi'
includes=[platform/'cores/esp32',platform/'variants'/options.target,sdk/memtype/'include',root/'src']
includes += [p/'src' if (p/'src').is_dir() else p for p in root.parent.iterdir() if p.is_dir() and (p/'library.properties').exists() and p!=root]
includes += [p/'src' for p in (platform/'libraries').iterdir() if (p/'src').is_dir()]
args=[str(compiler),'-c','@'+str(sdk/'flags/cpp_flags'),'-Os','-Werror=return-type','-fmax-errors=5','-DF_CPU=240000000L','-DARDUINO=10600','-DARDUINO_ARCH_ESP32','-DESP32=ESP32','-DCORE_DEBUG_LEVEL=4','-DARDUINO_RUNNING_CORE=1','-DARDUINO_EVENT_RUNNING_CORE=1','-DBOARD_HAS_PSRAM','-DARDUINO_USB_MODE=1','-DARDUINO_USB_CDC_ON_BOOT=0','-DDISPLAYWX_CONFIG_HEADER="'+configfile.as_posix()+'"','@'+str(sdk/'flags/defines'),'-iprefix',str(sdk/'include')+'/', '@'+str(sdk/'flags/includes')]
args += ['-I'+str(p) for p in includes]
(out/'compiler-command.json').write_text(json.dumps(args,indent=2),encoding='utf-8')
sources = sorted(p for p in (root/'src').rglob('*') if p.suffix in ('.cpp','.c'))
if options.filter: sources=[p for p in sources if any(term in p.as_posix() for term in options.filter.split(','))]
if not sources: parser.error('No source matched --filter')
def compile_source(p):
    stem = str(p.relative_to(root)).replace('/', '_').replace(chr(92), '_')
    cmd = args.copy()
    if p.suffix == '.c':
        cmd[0] = cmd[0].replace('g++.exe','gcc.exe')
        cmd[cmd.index('@'+str(sdk/'flags/cpp_flags'))] = '@'+str(sdk/'flags/c_flags')
    cmd += [str(p), '-o', str(out/(stem+'.o'))]
    run = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='replace')
    log = run.stdout + run.stderr
    (out/(stem+'.log')).write_text(log, encoding='utf-8')
    return {'source':str(p.relative_to(root)), 'exit_code':run.returncode, 'log':log}
results=[]
with ThreadPoolExecutor(max_workers=4) as pool:
    for task in as_completed([pool.submit(compile_source,p) for p in sources]):
        r=task.result(); results.append(r)
        if r['exit_code']: print('FAIL '+r['source']+'\n'+r['log'],flush=True)
(out/'results.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
print(f"{name}: passed {sum(r['exit_code']==0 for r in results)}/{len(results)} source compilations",flush=True)
sys.exit(any(r['exit_code'] for r in results))
