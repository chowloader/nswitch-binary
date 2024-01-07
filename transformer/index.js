import { BufferUtility, FileSystemModule } from 'bufferutility';
import { readFileSync, writeFileSync } from 'fs';
import { join } from 'path';
import fetch from 'node-fetch';
import macho from 'macho';

import * as url from 'url';
const __dirname = url.fileURLToPath(new URL('.', import.meta.url));

// config

let debug = true;

let native_funcs = {
  _JS_SetPropertyStr: 0xBADF670,
  _JS_NewObject: 0xBAD6000,
  _JS_NewCFunction2: 0xBAD6110,
  _native_qjs_malloc: 0xBBA9330,
  _native_qjs_free: 0xBBA94D0,
  _native_qjs_realloc: 0xBBA9550,
  _operator_new: 0xBCDD470,
  _JS_ToCStringLen2: 0xBAD51E0,
  _JS_NewStringLen: 0xBAD3250,
  _strlen: 0xBCDD060,
  _JS_FreeCString: 0xBAD5570,
  _BaseFile_initialize: 0xBC5C9AC,
  _BaseFile_get_size: 0xBC5CCB4,
  _BaseFile_read: 0xBC5D040,
  _BaseFile_close: 0xBC5CC4C,
  _stbi_info_from_memory: 0x808C1F0,
  _ChowdrenCacheImage: 0x809A660,
  _operator_delete: 0xBCDCFF0,
  _get_aot_object: 0xBB9D940,
  _JS_GetGlobalObject: 0xBAD7D10,
  _JS_GetPropertyStr: 0xBADB890,
  _JS_NewArray: 0xBAD5FE0,
  _JS_DefinePropertyValueUint32: 0xBADFD90,
  _js_array_join: 0xBB8A860,
  _JS_ExecutePendingJob: 0xBACEAB0,
  _js_global_decodeURI: 0xBB8E250,
  _strcpy: 0xBCDD770,
  _strcmp: 0xBCDD2E0,
  _init_aot: 0xBB07720,
  _JS_Call: 0xBAF3C30,
  _platform_begin_draw: 0xBC5A6F0,
  _Render_set_view: 0xBC5E770,
  _Render_clear: 0xBC5FC70,
  _ImageUtils_update: 0x809A9B0,
  _platform_swap_buffers: 0xBC5A930,
  _FontUtils_parse_color: 0x80B2030,
  _JS_ToInt32: 0xBAE25B0,
  _JS_ToInt64: 0xBAE22E0,
  _Thread_start: 0x8005254,
  _Thread_isNull: 0xBCDD0B0,
  _Thread_join: 0xBCDD120,
  _Thread_detach: 0xBCDD440,
  _Thread_delete: 0xBCDD0D0,
  _platform_sleep: 0xBC59A20,
  _nn_os_GetCurrentThread: 0xBCDD170,
  _nn_os_SetThreadCoreMask: 0xBCDD180,
  _JS_NewRuntime: 0xBACE940,
  _JS_NewContext: 0xBACFD70,
  _JS_SetCanBlock: 0xBACE980,
  _JS_FreeContext: 0xBAD2980,
  _JS_FreeRuntime: 0xBACE690,
  _cmemcpy: 0xBCDD070,
  _JS_Eval: 0xBAF6DD0,
  _JS_GetOpaque: 0xBAE1580,
  _get_cached_image: 0x80AF860,
  _JS_Throw: 0xBAD7D40,
  _build_backtrace: 0xBAF4D00,
  _std_string_append: 0xBCDD500,
  _stb_vorbis_open_memory: 0x80BF850,
  _ChowdrenPreloadAudio: 0x80D0300,
  _chowdren_main: 0x8089BB0,
  _JS_ToFloat64: 0xBAE1B70,
  _ImageUtils_get_image: 0x808C820,
  _JS_NewObjectClass: 0xBAD5E50,
  _JS_NewClassID: 0xBAD4730,
  _JS_NewClass: 0xBAD4790,
  _JS_SetOpaque: 0xBAD66B0
}

let native_datas = {
  _isDrawing: 0xBF27355,
  _Render_offset: 0x10542300,
  _Render_offsetf: 0x10542348,
  _ChowJSRuntime: 0xC136AC0,
  _ChowJSContext: 0xC136AB8,
  _CanvasClassID: 0xC136A8C,
  _JSVALOffset: 0xC131E10,
}

let entry_funcs = { // Calls address
  _initChowLoader: { forward: true, addr: 0xBC5E1A0 },
  _initChowLoaderObject: { forward: false, addr: 0xBBAA0EC },
  _initAOT: { forward: false, addr: 0xBBAA290 },
  _hookJSAOT: { forward: false, addr: 0xBB3FDF8 },
  _hookJSVARREF: { forward: false, addr: 0xBB3FEF8 },
  _hookJSVAL: { forward: false, addr: 0xBB3FF78 },
  _hookBuildBacktrace: { forward: false, addr: [ 0xBAF2F1C, 0xBAF56F0, 0xBAF5888, 0xBB053D4, 0xBB75CE0 ] },
  _hookThrow: { forward: false, addr: 0xA3DEEFC },
  _hookFonts: { forward: false, addr: 0x80B1398 },
  _hookBorder: { forward: false, addr: 0xBB9FD58 },
}

let qjs_mem_funcs = 0xBF13640;

let base_addr = 0xBAB3FE8;
let base_data = 0xC13AA00;

let data_buffer = Buffer.alloc(0);

let custom_patches = [
  "000CC67C F5FFFF17" // Finish a function that has been misaligned
];

(async () => {

let buf = new BufferUtility(join(__dirname, "..", "chowloader.o"), {
  module: FileSystemModule
});

let file = macho.parse(readFileSync(join(__dirname, "..", "chowloader.o")));
let symtab = file.cmds.find(c => c.type === "symtab");
let TEXT = file.cmds.find(c => c.name === "__TEXT");
let text = TEXT.sections.find(c => c.sectname === "__text");
let cstring = TEXT.sections.find(c => c.sectname === "__cstring");
let dataBuffer = Buffer.concat(file.cmds.find(c => c.name === "__DATA").sections.map(c => c.data));

buf.position = symtab.stroff;

let symbols = [];
buf.position = symtab.symoff;
for(let i = 0; i < symtab.nsyms; i++){
  let sym = {};
  sym.n_strx = buf.readUInt32();
  sym.n_type = buf.readUInt8(); // N_SECT | N_EXT
  sym.n_sect = buf.readUInt8();
  sym.n_desc = buf.readUInt16();
  sym.n_value = buf.readUInt64();
  symbols.push(sym);
}
for(let sym of symbols){
  buf.position = symtab.stroff + sym.n_strx;
  let str = "";
  while(true){
    let byte = buf.readByte();
    if(byte !== 0){
      str += String.fromCharCode(byte);
    } else break;
  }
  sym.str = str;
}
let header = symbols.find(c => c.str === "__mh_execute_header");
let mask = header.n_value - 1;
symbols = symbols.filter(c => c.str !== "__mh_execute_header");
for(let sym of symbols){
  sym.n_value = sym.n_value & mask;
}

symbols.sort((a,b) => (a.n_value - b.n_value));

let dataOffset = 0;
let firstData = true;
let functions = [];
let datas = [];
for(let i = 0; i < symbols.length; i++){
  if(symbols[i].n_sect > 3){
    if(firstData){
      firstData = false;
      dataOffset = symbols[i].n_value;
    }
    datas.push({
      name: symbols[i].str,
      offset: symbols[i].n_value - dataOffset
    });
  } else if(symbols[i].n_sect === 1 && symbols[i].str !== "") {
    let func = {};
    func.name = symbols[i].str;
    func.start = symbols[i].n_value;
    if(symbols[i+1]){
      func.end = symbols[i+1].n_value;
    } else {
      func.end = text.offset + text.size;
    }
    buf.position = func.start;
    func.data = Buffer.from(buf.readBytes(func.end - func.start));
    if(func.data.length === 0) continue;
    functions.push(func);
  }
}

for(let i = 0; i < datas.length; i++){
  let end = datas[i+1]?.offset;
  if(!end) end = dataBuffer.length;
  datas[i].size = end - datas[i].offset;
}

let base_functions = [...functions];

functions = functions.filter(c => !["_main", ...Object.keys(native_funcs)].includes(c.name));

let base = base_addr;
let data_base = base_data;

for(let func of functions){
  func.base_addr = base;
  base += func.data.length;
}

functions.sort((a,b) => (a.base_addr - b.base_addr));

function parseNumber(ins){
  let num = BigInt(ins.split("#")[1].split("]")[0]);
  if(num < 0x8000000000000000n){
    return Number(num);
  } else {
    return Number(num - (1n << 64n));
  }
}

function findByOffset(offset){
  for(let f of base_functions){ // FUNC
    if(f.start === offset){
      if(Object.keys(native_funcs).includes(f.name)){
        return native_funcs[f.name];
      } else {
        return functions.find(a => a.name === f.name).base_addr;
      }
    }
  }
  if(offset > cstring.offset + cstring.size){ // DATA
    let data = datas.find(c => c.offset === offset - dataOffset);

    let data_off;
    if(Object.keys(native_datas).includes(data.name)){
      data_off = native_datas[data.name];
    } else if(data.new_offset){
      data_off = data.new_offset;
    } else {
      data.new_offset = data_off = data_base;
      data_base += data.size;
      data_buffer = Buffer.concat([data_buffer, dataBuffer.subarray(data.offset, data.offset + data.size)]);
    }

    return data_off;
  } else { // STRING
    return base + (offset - cstring.offset)
  }
}

async function getHexInstruction(instruction){
  return (await fetch("https://armconverter.com/api/convert", {
    method: "POST",
    body: JSON.stringify({
      "asm": instruction,
      "offset": "",
      "arch": ["arm64"]
    }),
    headers: {
      "Content-Type": "application/json"
    }
  }).then(res => res.json())).hex.arm64[1];
}

async function createBranch(func, instruction, forward = false){
  let new_offset = func - instruction;
  let ins = `${forward ? "B" : "BL"} #${new_offset < 0 ? '-' : ''}0x${new_offset.toString(16).replace('-', '')}`;
  return await getHexInstruction(ins);
}

for(let func of functions){
  let instructions_str = func.data.toString("hex").toUpperCase();
  let instructions = instructions_str.match(/.{1,8}/g);

  let raw_instructions = (await fetch("https://armconverter.com/api/convert", {
    method: "POST",
    body: JSON.stringify({
      "hex": instructions_str,
      "offset": "",
      "arch": ["arm64"]
    }),
    headers: {
      "Content-Type": "application/json"
    }
  }).then(res => res.json())).asm.arm64[1].split("\n");

  for(let i = 0; i < instructions.length; i++){
    let ins = {};
    ins.hex = instructions[i];
    ins.str = raw_instructions[i];
    ins.type = raw_instructions[i].split(" ")[0].toUpperCase();
    ins.offset = i * 4;

    instructions[i] = ins;
  }

  func.instructions = instructions;

  let base_func = base_functions.find(f => f.name === func.name);

  for(let i = 0; i < instructions.length; i++){
    if(instructions[i].type === "BL"){
      instructions[i].hex = await createBranch(findByOffset(base_func.start + parseNumber(instructions[i].str)), func.base_addr + i * 4, false);
    } else if(instructions[i].type === "ADRP"){
      let adrp = instructions[i];
      let next_ins = instructions[++i];

      let register = adrp.str.split(",")[0].split(" ")[1];
      let process_adrp = v => Number(BigInt(v) & 0xFFFFFFFFFFFFF000n);

      let ins;

      if(next_ins.type === "ADD"){
        let new_offset = findByOffset(process_adrp(base_func.start + i * 4) + parseNumber(adrp.str) + parseNumber(next_ins.str));
  
        let adrp_offset = process_adrp(new_offset) - process_adrp(func.base_addr + i * 4);
        ins = `ADRP ${register}, #${adrp_offset < 0 ? '-' : ''}0x${adrp_offset.toString(16).replace('-', '')}\nADD ${register}, ${register}, #0x${(new_offset & 0xFFF).toString(16)}`;
      } else { // DATA
        let data_off;
        if(next_ins.str.indexOf("#") != -1) data_off = parseNumber(next_ins.str);
        else data_off = 0;

        let data = datas.find(c => c.offset === data_off);

        let val_register = next_ins.str.split(",")[0].split(" ")[1];

        if(Object.keys(native_datas).includes(data.name)){
          data_off = native_datas[data.name];
        } else if(data.new_offset){
          data_off = data.new_offset;
        } else {
          data.new_offset = data_off = data_base;
          data_base += data.size;
          data_buffer = Buffer.concat([data_buffer, dataBuffer.subarray(data.offset, data.offset + data.size)]);
        }
        
        let adrp_offset = process_adrp(data_off) - process_adrp(func.base_addr + i * 4);
        let final_offset = data_off & 0xFFF;

        ins = `ADRP ${register}, #${adrp_offset < 0 ? '-' : ''}0x${adrp_offset.toString(16).replace('-', '')}\n`

        if(final_offset !== 0) ins += `${next_ins.type} ${val_register}, [${register}, #0x${final_offset.toString(16)}]`;
        else ins += `${next_ins.type} ${val_register}, [${register}]`;
      }

      let hex = (await getHexInstruction(ins)).split("\n");
      adrp.hex = hex[0];
      next_ins.hex = hex[1];
    }
  }

  let ins_hex = "";

  for(let ins of instructions){
    ins_hex += ins.hex;
  }

  func.data = Buffer.from(ins_hex, "hex");
}

if(debug){
  console.log('Functions:');
  for(let func of functions){
    console.log(`${func.name}: 0x${func.base_addr.toString(16).toUpperCase()}`);
  }
  console.log('\nDatas:');
  for(let data of datas.filter(d => d.new_offset).sort((a,b)=>(a.new_offset-b.new_offset))){
    console.log(`${data.name}: 0x${data.new_offset.toString(16).toUpperCase()}`)
  }
}

function toIPSAddr(num){
  return ((typeof num === "string" ? Number(`0x${num}`) : num) - 0x8004000).toString(16).padStart(8, "0").toUpperCase();
}

function toPointerAddr(num){
  return ((typeof num === "string" ? Number(`0x${num}`) : num) - 0x8004000).toString(16).padStart(16, "0").toUpperCase().match(/.{1,2}/g).reverse().join("");
}

custom_patches.push(`${toIPSAddr(qjs_mem_funcs)} ${toPointerAddr(functions.find(c => c.name === "_qjs_malloc").base_addr)}`);
custom_patches.push(`${toIPSAddr(qjs_mem_funcs+8)} ${toPointerAddr(functions.find(c => c.name === "_qjs_free").base_addr)}`);
custom_patches.push(`${toIPSAddr(qjs_mem_funcs+16)} ${toPointerAddr(functions.find(c => c.name === "_qjs_realloc").base_addr)}`);

let final_buf = Buffer.concat([...functions.map(f => f.data), cstring.data]);

function splitChunk(base, buf){
  let addr = [];
  let patch = buf.toString("hex").toUpperCase();
  let bytelen = 8;
  for(let i = 0; i < Math.trunc(buf.length / bytelen); i++){
    addr.push(`${toIPSAddr(base)} ${patch.substring(0, bytelen * 2)}`);
    base += bytelen;
    patch = patch.substring(8*2);
  }
  if(patch !== "") addr.push(`${toIPSAddr(base)} ${patch}`);
  return addr.join("\n");
}

let pchtxt = `@nsobid-318E91DDED917C19DEE42932587DA4C4AD83CB68

@flag offset_shift 0x100

@enabled
${splitChunk(base_addr, final_buf) + (data_buffer.length > 0 ? "\n" + splitChunk(base_data, data_buffer) : "")}
${await (async () => {
  let entries = [];
  for(let entry_name of Object.keys(entry_funcs)){
    let entry = functions.find(f => f.name === entry_name);
    if(entry == null) continue;
    let e = entry_funcs[entry_name];
    if(Array.isArray(e.addr)){
      for(let addr of e.addr){
        entries.push(toIPSAddr(addr) + " " + await createBranch(entry.base_addr, addr, e.forward));
      }
    } else {
      entries.push(toIPSAddr(e.addr) + " " + await createBranch(entry.base_addr, e.addr, e.forward));
    }
  }
  return entries.join("\n");
})()}
${custom_patches.join("\n")}
@stop`;

writeFileSync(join(__dirname, "main.pchtxt"), pchtxt, "utf8");

})()