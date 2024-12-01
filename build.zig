const std = @import("std");
const mem = std.mem;
const elf = std.elf;
const Writer = std.fs.File.Writer;
const SymHashMap = std.StringHashMap(elf.Sym);

const optimize: std.builtin.OptimizeMode = .ReleaseFast;

const GAME_VERSION = "1.0.3";
const NSO_BUILD_ID = "318E91DDED917C19DEE42932587DA4C4AD83CB68";

const BASE_OFFSET = 0x8004000;

const PatchType = enum {
  Branch,
  BranchLink,
  StaticData
};

const Patches = struct {
  symbolName: []const u8, // Name of the redirect target function
  patchType: PatchType,
  addresses: []const u32 // Addresses to replace
};

const patches = [_]Patches{
  // Patch Branch Instructions
  Patches{
    .symbolName = "initChowLoader",
    .patchType = .Branch,
    .addresses = &[_]u32{ 0xBC5E1A0 }
  },
  Patches{
    .symbolName = "initChowLoaderObject",
    .patchType = .BranchLink,
    .addresses = &[_]u32{ 0xBBAA0EC }
  },
  Patches{
    .symbolName = "initAOT",
    .patchType = .BranchLink,
    .addresses = &[_]u32{ 0xBBAA290 }
  },
  Patches{
    .symbolName = "hookJSAOT",
    .patchType = .BranchLink,
    .addresses = &[_]u32{ 0xBB3FDF8 }
  },
  Patches{
    .symbolName = "hookJSVARREF",
    .patchType = .BranchLink,
    .addresses = &[_]u32{ 0xBB3FEF8 }
  },
  Patches{
    .symbolName = "hookJSVAL",
    .patchType = .BranchLink,
    .addresses = &[_]u32{ 0xBB3FF78 }
  },
  Patches{
    .symbolName = "hookBuildBacktrace",
    .patchType = .BranchLink,
    .addresses = &[_]u32{ 0xBAF2F1C, 0xBAF56F0, 0xBAF5888, 0xBB053D4, 0xBB75CE0 }
  },
  Patches{
    .symbolName = "hookThrow",
    .patchType = .BranchLink,
    .addresses = &[_]u32{ 0xA3DEEFC }
  },
  Patches{
    .symbolName = "hookFonts",
    .patchType = .BranchLink,
    .addresses = &[_]u32{ 0x80B1398 }
  },
  Patches{
    .symbolName = "hookBorder",
    .patchType = .BranchLink,
    .addresses = &[_]u32{ 0xBB9FD58 }
  },

  // Patch Static Data Pointers
  Patches{
    .symbolName = "qjs_malloc",
    .patchType = .StaticData,
    .addresses = &[_]u32{ 0xBF13640 }
  },
  Patches{
    .symbolName = "qjs_free",
    .patchType = .StaticData,
    .addresses = &[_]u32{ 0xBF13648 }
  },
  Patches{
    .symbolName = "qjs_realloc",
    .patchType = .StaticData,
    .addresses = &[_]u32{ 0xBF13650 }
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

  inline for(patches) |patch| {
    patchBin.forceUndefinedSymbol(patch.symbolName); // Emit symbol on debug info on Releases optimize modes
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

  const installArtifact = b.addInstallArtifact(patchBin, .{ .dest_dir = .{ .override = .{ .custom = "../build" } } });

  step.dependOn(&installArtifact.step);

  b.getInstallStep().dependOn(step);
}

const ELFSections = struct {
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

  const binData = try bin.readToEndAlloc(allocator, std.math.maxInt(usize));
  defer allocator.free(binData);

  var binStream = std.io.fixedBufferStream(binData);

  const binReader = binStream.reader();

  const elfHeader = try elf.Header.read(&binStream);

  binStream.pos = elfHeader.shoff + elfHeader.shstrndx * @sizeOf(elf.Elf64_Shdr);

  var sections: ELFSections = undefined;
  sections.shstrtab = try binReader.readStruct(elf.Elf64_Shdr);

  var sectionsIterator = elfHeader.section_header_iterator(&binStream);

  while (sectionsIterator.next() catch null) |section| {
    const sectionName = getStrName(binData, sections.shstrtab.sh_offset, section.sh_name);
    const typeInfo = @typeInfo(ELFSections).Struct.fields;
    inline for (typeInfo) |field| {
      if (std.mem.eql(u8, sectionName, "." ++ field.name)) {
        @field(sections, field.name) = section;
      }
    }
  }

  std.debug.print("Base ASM Address: 0x{X}\n", .{ sections.text.sh_addr });
  std.debug.print("Base Data Address: 0x{X}\n\n", .{ sections.data.sh_addr });

  // Reading symbols and showing their addresses for debugging

  var symbols = SymHashMap.init(allocator);
  defer symbols.deinit();

  binStream.pos = sections.symtab.sh_offset;

  const symbolsCount = @divTrunc(sections.symtab.sh_size, @sizeOf(elf.Sym));
  try symbols.ensureTotalCapacity(@intCast(symbolsCount));

  for (0..symbolsCount) |_| {
    const symbol: elf.Sym = try binReader.readStruct(elf.Sym);
    const symbolName = getStrName(binData, sections.strtab.sh_offset, symbol.st_name);

    try symbols.put(symbolName, symbol);

    if (symbol.st_type() == elf.STB_WEAK) { // Functions
      std.debug.print("{s} (func): 0x{X}\n", .{ symbolName, symbol.st_value });
    } else if (symbol.st_type() == elf.STB_GLOBAL) { // Data
      std.debug.print("{s} (data): 0x{X}\n", .{ symbolName, symbol.st_value });
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

    try writeSectionIPS(writer, sections.text.sh_addr - BASE_OFFSET + NSO_HEADER_OFFSET, getSectionData(binData, sections.text));
    try writeSectionIPS(writer, sections.data.sh_addr - BASE_OFFSET + NSO_HEADER_OFFSET, getSectionData(binData, sections.data));

    for(patches) |patch| {
      if(symbols.get(patch.symbolName)) |symbol| {
        for(patch.addresses) |address| {
          try writer.writeInt(u32, address - BASE_OFFSET + NSO_HEADER_OFFSET, .big);
          switch(patch.patchType){
            .Branch, .BranchLink => {
              const off: i28 = @intCast(@as(isize, @intCast(symbol.st_value)) - @as(isize, @intCast(address)));
              const inst = if(patch.patchType == .Branch) createARM64Branch(off) else createARM64BranchLink(off);

              try writer.writeInt(u16, 4, .big);
              try writer.writeInt(u32, inst, .big);
            },
            .StaticData => {
              try writer.writeInt(u16, 8, .big);
              try writer.writeInt(u64, symbol.st_value - BASE_OFFSET, .little);
            }
          }
        }
      } else {
        std.debug.print("Can't find the symbol for {s}!!!\n", .{ patch.symbolName });
        return error.SymbolNotFound;
      }
    }

    try writer.writeAll("EEOF"); // FOOT MAGIC
  }

  std.debug.print("Building PCHTXT...\n", .{});

  { // Create PCHTXT file
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

    try writeSectionPCHTXT(writer, sections.text.sh_addr - BASE_OFFSET, getSectionData(binData, sections.text));
    try writeSectionPCHTXT(writer, sections.data.sh_addr - BASE_OFFSET, getSectionData(binData, sections.data));

    for(patches) |patch| {
      if(symbols.get(patch.symbolName)) |symbol| {
        for(patch.addresses) |address| switch(patch.patchType) {
          .Branch, .BranchLink => {
            const off: i28 = @intCast(@as(isize, @intCast(symbol.st_value)) - @as(isize, @intCast(address)));
            const inst = if(patch.patchType == .Branch) createARM64Branch(off) else createARM64BranchLink(off);

            try writer.print("{X:0>8} {X:0>8}\n", .{ address - BASE_OFFSET, inst });
          },
          .StaticData => {
            try writer.print("{X:0>8} {X:0>16}\n", .{ address - BASE_OFFSET, @byteSwap(symbol.st_value - BASE_OFFSET) });
          }
        };
      } else {
        std.debug.print("Can't find the symbol for {s}!!!\n", .{ patch.symbolName });
        return error.SymbolNotFound;
      }
    }

    try writer.writeAll("@stop");
  }

  std.debug.print("\nSUCCESS!\n", .{});
}

fn getStrName(data: []const u8, sectionOffset: usize, nameOffset: usize) []const u8 {
  const buf = data[sectionOffset + nameOffset ..];
  const len = std.mem.indexOfScalar(u8, buf, 0).?;
  return buf[0..len];
}

fn getSectionData(elfData: []const u8, section: elf.Elf64_Shdr) []const u8 {
  return elfData[section.sh_offset .. section.sh_offset + section.sh_size];
}

const SectionChunkLen = 0x8;

fn writeSectionIPS(writer: Writer, sectionOffset: u64, sectionData: []const u8) !void {
  const chunkCount = @divTrunc(sectionData.len, SectionChunkLen);
  for(0..chunkCount) |i| {
    const chunkOffset = i * SectionChunkLen;
    try writer.writeInt(u32, @intCast(sectionOffset + chunkOffset), .big);
    try writer.writeInt(u16, SectionChunkLen, .big);
    try writer.writeAll(sectionData[chunkOffset .. chunkOffset + SectionChunkLen]);
  }

  const finalBytesLength = sectionData.len - chunkCount * SectionChunkLen;
  if(finalBytesLength > 0){
    const chunkOffset = chunkCount * SectionChunkLen;
    try writer.writeInt(u32, @intCast(sectionOffset + chunkOffset), .big);
    try writer.writeInt(u16, @intCast(finalBytesLength), .big);
    try writer.writeAll(sectionData[chunkOffset .. chunkOffset + finalBytesLength]);
  }
}

fn writeSectionPCHTXT(writer: Writer, sectionOffset: u64, sectionData: []const u8) !void {
  const chunkCount = @divTrunc(sectionData.len, SectionChunkLen);
  for(0..chunkCount) |i| {
    const chunkOffset = i * SectionChunkLen;
    try writer.print("{X:0>8} ", .{ sectionOffset + chunkOffset });
    for(0..SectionChunkLen) |byteIndex| {
      try writer.print("{X:0>2}", .{ sectionData[chunkOffset + byteIndex] });
    }
    try writer.writeAll("\n");
  }

  const finalBytesLength = sectionData.len - chunkCount * SectionChunkLen;
  if(finalBytesLength > 0){
    const chunkOffset = chunkCount * SectionChunkLen;
    try writer.print("{X:0>8} ", .{ sectionOffset + chunkOffset });
    for(0..finalBytesLength) |byteIndex| {
      try writer.print("{X:0>2}", .{ sectionData[chunkOffset + byteIndex] });
    }
    try writer.writeAll("\n");
  }
}

// Arm® Architecture Reference Manual for A-profile architecture
// Section C4.1: A64 instruction set encoding
// https://developer.arm.com/documentation/ddi0487/latest/

// Section C6.2.26: B

const Inst_B = packed struct(u32) {
  imm26: i26,
  reserved: u6 = 0b000101,
};

fn createARM64Branch(pcOffset: i28) u32 {
  const inst = Inst_B{ .imm26 = @intCast(pcOffset >> 2) };
  return std.mem.readInt(u32, std.mem.asBytes(&inst), .big);
}

// Section C6.2.35: BL

const Inst_BL = packed struct(u32) {
  imm26: i26,
  reserved: u6 = 0b100101,
};

fn createARM64BranchLink(pcOffset: i28) u32 { // C6.2.35
  const inst = Inst_BL{ .imm26 = @intCast(pcOffset >> 2) };
  return std.mem.readInt(u32, std.mem.asBytes(&inst), .big);
}
