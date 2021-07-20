# msitools

Note: This is a temporary fork of the awesome https://gitlab.gnome.org/GNOME/msitools to add support to
use Environment Variables in Wix configuration. Once this issue is resolved in the origin repo, this repo may be deleted/archived. 

## Introduction

*msitools* is a set of programs to inspect and build Windows Installer
(.MSI) files.  It is based on `libmsi`, a portable library to read and
write .MSI files.  libmsi in turn is a port of (and a subset of)
[Wine](https://www.winehq.org/)'s implementation of the Windows
Installer.

*msitools* plans to be a solution for packaging and deployment of
cross-compiled Windows applications.

## Build from Source

*Prerequisites:* meson, bison. 

```bash
meson setup builddir
meson compile 
```

Note: If you run into this error:
```bash
FAILED: libmsi/sql-parser.c libmsi/sql-parser.h
/usr/bin/bison ../libmsi/sql-parser.y -y -Wno-yacc --defines -o libmsi/sql-parser.c
/Library/Developer/CommandLineTools/usr/bin/bison: invalid option -- W
Try `/Library/Developer/CommandLineTools/usr/bin/bison --help' for more information.
[4/56] Generating libmsi-enums.c with a custom command (wrapped by meson to capture output)
ninja: build stopped: subcommand failed.
```

when building in Macos. Ensure you `brew install bison` and use that instead of the one at `usr/bin/bison`. 

## Tools

Provided tools include:

- `msiinfo`, to inspect MSI files

- `msibuild`, a low-level tool to create MSI files

- `msidiff`, compares contents of two MSI files with diff

- `msidump`, dumps raw MSI tables and stream content

- `msiextract`, to inspect and extract the files of an MSI file

- `wixl`, a WiX-like tool, that builds Windows Installer (MSI)
  packages from an XML document, and tries to share the same syntax as
  the [WiX toolset](http://wixtoolset.org/)

- `wixl-heat`, a tool that builds XML fragments from a list of files
  and directories.


## Notes

Right now, *msitools* does not work under Windows.  It is planned that
it will self-host.

While in a very early stage, it is already usable.

*msitools* uses [libgsf](https://gitlab.gnome.org/GNOME/libgsf) in
order to read OLE Structured Storage files (which are the underlying
format of .MSI files).

Wixl lacks many features compared to WiX. As always, contributions
are welcome!

Reporting issues and sending pull requests is welcome!
