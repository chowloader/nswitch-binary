const std = @import("std");
const mem = std.mem;
const elf = std.elf;
const Writer = std.fs.File.Writer;
const SymHashMap = std.StringHashMap(elf.Sym);

const optimize: std.builtin.OptimizeMode = .ReleaseFast;

const GAME_VERSION = "1.0.3";
const NSO_BUILD_ID = "318E91DDED917C19DEE42932587DA4C4AD83CB68";

const BASE_OFFSET = 0x8004000;

const PatchFuncs = struct {
  symbol: []const u8, // Name of the redirect target function
  is_linked: bool = true, // true for BL inst, false for B inst
  is_ptr: bool = false, // Directly patch pointer on static data sections
  addrs: []const u32 // Addresses of branch instructions to replace
};

const patchFuncs = [_]PatchFuncs{
  // Patch Branch Instructions
  PatchFuncs{
    .symbol = "initChowLoader",
    .is_linked = false,
    .addrs = &[_]u32{ 0xBC5E1A0 }
  },
  PatchFuncs{
    .symbol = "initChowLoaderObject",
    .is_linked = true,
    .addrs = &[_]u32{ 0xBBAA0EC }
  },
  PatchFuncs{
    .symbol = "initAOT",
    .is_linked = true,
    .addrs = &[_]u32{ 0xBBAA290 }
  },
  PatchFuncs{
    .symbol = "hookJSAOT",
    .is_linked = true,
    .addrs = &[_]u32{ 0xBB3FDF8 }
  },
  PatchFuncs{
    .symbol = "hookJSVARREF",
    .is_linked = true,
    .addrs = &[_]u32{ 0xBB3FEF8 }
  },
  PatchFuncs{
    .symbol = "hookJSVAL",
    .is_linked = true,
    .addrs = &[_]u32{ 0xBB3FF78 }
  },
  PatchFuncs{
    .symbol = "hookBuildBacktrace",
    .is_linked = true,
    .addrs = &[_]u32{ 0xBAF2F1C, 0xBAF56F0, 0xBAF5888, 0xBB053D4, 0xBB75CE0 }
  },
  PatchFuncs{
    .symbol = "hookThrow",
    .is_linked = true,
    .addrs = &[_]u32{ 0xA3DEEFC }
  },
  PatchFuncs{
    .symbol = "hookFonts",
    .is_linked = true,
    .addrs = &[_]u32{ 0x80B1398 }
  },
  PatchFuncs{
    .symbol = "hookBorder",
    .is_linked = true,
    .addrs = &[_]u32{ 0xBB9FD58 }
  },

  // Patch Static Data Pointers
  PatchFuncs{
    .symbol = "qjs_malloc",
    .is_ptr = true,
    .addrs = &[_]u32{ 0xBF13640 }
  },
  PatchFuncs{
    .symbol = "qjs_free",
    .is_ptr = true,
    .addrs = &[_]u32{ 0xBF13648 }
  },
  PatchFuncs{
    .symbol = "qjs_realloc",
    .is_ptr = true,
    .addrs = &[_]u32{ 0xBF13650 }
  },
};

pub fn build(b: *std.Build) !void {
  // Target Tegra X1
  const target = b.resolveTargetQuery(.{
    .abi = .eabi,
    .cpu_arch = .aarch64,
    .cpu_model = .{ .explicit = &std.Target.arm.cpu.cortex_a57 },
    .ofmt = .elf,
    .os_tag = .freestanding
  });

  // Create the ELF binary
  const patchBin = b.addExecutable(.{
    .name = "chowloader.elf",
    .target = target,
    .optimize = optimize,
    .pic = false, // Disable Position Independent Code
    .strip = false // Emit debug info
  });

  patchBin.pie = false; // Disable Position Independent Executable
  patchBin.want_lto = true; // Reduce bundle size on Debug optimize mode
  patchBin.entry = .disabled;

  inline for(patchFuncs) |patchFunc| {
    patchBin.forceUndefinedSymbol(patchFunc.symbol); // Emit symbol on debug info on Releases optimize modes
  }

  patchBin.setLinkerScript(b.path("linker/" ++ GAME_VERSION ++ ".ld"));

  patchBin.addCSourceFiles(.{ .files = &.{
    "chowloader.c",
    "utils.c",
    "lib/chowjs.c",
    "chowloader/aot.c",
    "chowloader/assets.c",
    "chowloader/error.c",
    "chowloader/renderer.c",
    "chowloader/thread.c",
  }, .root = b.path("src") });

  patchBin.addIncludePath(b.path("src"));
  patchBin.addIncludePath(b.path("src/lib"));
  patchBin.addIncludePath(b.path("src/chowloader"));

  // Adding patch files step
  const step = b.step("build-extra", "Build the ChowLoader IPS and PCHTXT");
  step.makeFn = buildIpsAndPchtxt;

  const install = b.addInstallArtifact(patchBin, .{ .dest_dir = .{ .override = .{ .custom = "../build" } } });

  step.dependOn(&install.step);

  b.getInstallStep().dependOn(step);
}

const Sections = struct {
  text: elf.Elf64_Shdr,
  data: elf.Elf64_Shdr,
  shstrtab: elf.Elf64_Shdr,
  strtab: elf.Elf64_Shdr,
  symtab: elf.Elf64_Shdr,
};

fn buildIpsAndPchtxt(step: *std.Build.Step, _: std.Progress.Node) anyerror!void {
  const allocator = step.owner.allocator;
  const cwd = std.fs.cwd();

  // Delete build files if an error occur
  errdefer blk: {
    var buildDir = cwd.openDir("build", .{ .iterate = true }) catch break :blk;
    defer buildDir.close();

    var it = buildDir.iterate();

    while(it.next() catch null) |entry| {
      buildDir.deleteFile(entry.name) catch {};
    }
  }

  // Reading elf sections
  
  std.debug.print("Creating patch files for OMORI {s} ({s})\n\n", .{ GAME_VERSION, NSO_BUILD_ID });

  var bin = try cwd.openFile("build/chowloader.elf", .{ .mode = .read_only });
  defer bin.close();

  const data = try bin.readToEndAlloc(allocator, std.math.maxInt(usize));
  defer allocator.free(data);

  var data_stream = std.io.fixedBufferStream(data);

  const reader = data_stream.reader();

  const header = try elf.Header.read(&data_stream);

  data_stream.pos = header.shoff + header.shstrndx * @sizeOf(elf.Elf64_Shdr);

  var sections: Sections = undefined;
  sections.shstrtab = try reader.readStruct(elf.Elf64_Shdr);

  var section_header_it = header.section_header_iterator(&data_stream);

  while (section_header_it.next() catch null) |section| {
    const name = getStrName(data, sections.shstrtab.sh_offset, section.sh_name);
    const type_info = @typeInfo(Sections).Struct.fields;
    inline for (type_info) |field| {
      if (std.mem.eql(u8, name, "." ++ field.name)) {
        @field(sections, field.name) = section;
      }
    }
  }

  std.debug.print("Base ASM Address: 0x{X}\n", .{sections.text.sh_addr});
  std.debug.print("Base Data Address: 0x{X}\n\n", .{sections.data.sh_addr});

  // Reading symbols and showing their addresses for debugging

  var symbols = SymHashMap.init(allocator);
  defer symbols.deinit();

  data_stream.pos = sections.symtab.sh_offset;

  const symbols_len = @divTrunc(sections.symtab.sh_size, @sizeOf(elf.Sym));
  try symbols.ensureTotalCapacity(@intCast(symbols_len));

  for (0..symbols_len) |_| {
    const sym: elf.Sym = try reader.readStruct(elf.Sym);
    const name = getStrName(data, sections.strtab.sh_offset, sym.st_name);

    try symbols.put(name, sym);

    if (sym.st_type() == elf.STB_WEAK) { // Functions
      std.debug.print("{s} (func): 0x{X}\n", .{ name, sym.st_value });
    } else if (sym.st_type() == elf.STB_GLOBAL) { // Data
      std.debug.print("{s} (data): 0x{X}\n", .{ name, sym.st_value });
    }
  }

  // Patch files building

  std.debug.print("\nBuilding IPS...\n", .{});

  { // Create IPS File
    const file = try cwd.createFile("build/" ++ NSO_BUILD_ID ++ ".ips", .{ .truncate = true });
    defer file.close();

    const writer = file.writer();

    const NSO_HEADER_OFFSET = 0x100;

    try writer.writeAll("IPS32"); // HEADER MAGIC

    try writeSectionIPS(writer, sections.text.sh_addr - BASE_OFFSET + NSO_HEADER_OFFSET, getData(data, sections.text));
    try writeSectionIPS(writer, sections.data.sh_addr - BASE_OFFSET + NSO_HEADER_OFFSET, getData(data, sections.data));

    for(patchFuncs) |entryFunc| {
      if(symbols.get(entryFunc.symbol)) |sym| {
        for(entryFunc.addrs) |addr| {
          if(addr == 0) break;
          if(entryFunc.is_ptr){
            try writer.writeInt(u32, addr - BASE_OFFSET + NSO_HEADER_OFFSET, .big);
            try writer.writeInt(u16, 8, .big);
            try writer.writeInt(u64, sym.st_value - BASE_OFFSET, .little);
          } else {
            const off: i28 = @intCast(@as(isize, @intCast(sym.st_value)) - @as(isize, @intCast(addr)));

            try writer.writeInt(u32, addr - BASE_OFFSET + NSO_HEADER_OFFSET, .big);
            try writer.writeInt(u16, 4, .big);
            try writer.writeInt(u32, createBranch(off, entryFunc.is_linked), .big);
          }
        }
      } else {
        std.debug.print("Can't find the symbol for {s}!!!\n", .{ entryFunc.symbol });
        return error.SymbolNotFound;
      }
    }

    try writer.writeAll("EEOF"); // FOOT MAGIC
  }

  std.debug.print("Building PCHTXT...\n", .{});

  {
    const file = try cwd.createFile("build/" ++ GAME_VERSION ++ ".pchtxt", .{ .truncate = true });
    defer file.close();

    const writer = file.writer();
    try writer.writeAll(
      "@nsobid-" ++ NSO_BUILD_ID ++ \\
      \\
      \\@flag offset_shift 0x100
      \\
      \\@enabled
      \\
    );

    try writeSectionPCHTXT(writer, sections.text.sh_addr - BASE_OFFSET, getData(data, sections.text));
    try writeSectionPCHTXT(writer, sections.data.sh_addr - BASE_OFFSET, getData(data, sections.data));

    for(patchFuncs) |entryFunc| {
      if(symbols.get(entryFunc.symbol)) |sym| {
        for(entryFunc.addrs) |addr| {
          if(addr == 0) break;
          if(entryFunc.is_ptr){
            try writer.print("{X:0>8} {X:0>16}\n", .{ addr - BASE_OFFSET, @byteSwap(sym.st_value - BASE_OFFSET) });
          } else {
            const off: i28 = @intCast(@as(isize, @intCast(sym.st_value)) - @as(isize, @intCast(addr)));
            try writer.print("{X:0>8} {X:0>8}\n", .{ addr - BASE_OFFSET, createBranch(off, entryFunc.is_linked) });
          }
        }
      } else {
        std.debug.print("Can't find the symbol for {s}!!!\n", .{ entryFunc.symbol });
        return error.SymbolNotFound;
      }
    }

    try writer.writeAll("@stop");
  }

  std.debug.print("\nSUCCESS!\n", .{});
}

fn getStrName(data: []const u8, offset: usize, index: usize) []const u8 {
  const buf = data[offset + index ..];
  const len = std.mem.indexOfScalar(u8, buf, 0).?;
  return buf[0..len];
}

fn getData(data: []const u8, sym: elf.Elf64_Shdr) []const u8 {
  return data[sym.sh_offset .. sym.sh_offset + sym.sh_size];
}

const BytesLenPerLine = 0x8;

fn writeSectionIPS(writer: Writer, offset: u64, data: []const u8) !void {
  const pair_end = @divTrunc(data.len, BytesLenPerLine);
  for(0..pair_end) |i| {
    const off = i * BytesLenPerLine;
    try writer.writeInt(u32, @intCast(offset + off), .big);
    try writer.writeInt(u16, BytesLenPerLine, .big);
    try writer.writeAll(data[off .. off + BytesLenPerLine]);
  }

  const diff = data.len - pair_end * BytesLenPerLine;
  if(diff > 0){
    const off = pair_end * BytesLenPerLine;
    try writer.writeInt(u32, @intCast(offset + off), .big);
    try writer.writeInt(u16, @intCast(diff), .big);
    try writer.writeAll(data[off .. off + diff]);
  }
}

fn writeSectionPCHTXT(writer: Writer, offset: u64, data: []const u8) !void {
  const pair_end = @divTrunc(data.len, BytesLenPerLine);
  for(0..pair_end) |i| {
    const off = i * BytesLenPerLine;
    try writer.print("{X:0>8} ", .{ offset + off });
    for(0..BytesLenPerLine) |bi| {
      try writer.print("{X:0>2}", .{ data[off + bi] });
    }
    try writer.writeAll("\n");
  }

  const diff = data.len - pair_end * BytesLenPerLine;
  if(diff > 0){
    const off = pair_end * BytesLenPerLine;
    try writer.print("{X:0>8} ", .{ offset + off });
    for(0..diff) |bi| {
      try writer.print("{X:0>2}", .{ data[off + bi] });
    }
    try writer.writeAll("\n");
  }
}

fn createBranch(offset: i28, is_linked: bool) u32 {
  return if(is_linked)
    createARM64BranchLink(offset)
  else
    createARM64Branch(offset);
}

// Arm® Architecture Reference Manual for A-profile architecture
// Section C4.1: A64 instruction set encoding
// https://developer.arm.com/documentation/ddi0487/latest/

// Section C6.2.26: B

const Inst_B = packed struct(u32) {
  imm26: i26,
  reserved: u6 = 0b000101,
};

fn createARM64Branch(offset: i28) u32 {
  const inst = Inst_B{ .imm26 = @intCast(offset >> 2) };
  return std.mem.readInt(u32, std.mem.asBytes(&inst), .big);
}

// Section C6.2.35: BL

const Inst_BL = packed struct(u32) {
  imm26: i26,
  reserved: u6 = 0b100101,
};

fn createARM64BranchLink(offset: i28) u32 { // C6.2.35
  const inst = Inst_BL{ .imm26 = @intCast(offset >> 2) };
  return std.mem.readInt(u32, std.mem.asBytes(&inst), .big);
}
