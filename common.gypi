{
  'variables': {
    'clang': 0,
    'openssl_no_asm': 1,
    'conditions': [
      ['OS=="mac" or OS=="linux"', {
        'clang': 1,
      }],
      ['OS=="win" and (MSVS_VERSION=="2013e" or MSVS_VERSION=="2012e" or MSVS_VERSION=="2010e")', {
        'msvs_express': 1,
      },{
        'msvs_express': 0,
      }],
    ],
    # Required by Linux (empty for now, should support it in future).
    'sysroot': '',
  },
  # Settings to compile node under Windows.
  'target_defaults': {
    'target_conditions': [
      ['_target_name.startswith("breakpad") or _target_name in ["crash_report_sender", "dump_syms"]', {
        'conditions': [
          ['OS=="mac"', {
            'xcode_settings': {
              'WARNING_CFLAGS': [
                '-Wno-deprecated-declarations',
                '-Wno-deprecated-register',
                '-Wno-unused-private-field',
                '-Wno-unused-function',
              ],
            },
          }],  # OS=="mac"
          ['OS=="linux"', {
            'cflags': [
              '-Wno-empty-body',
            ],
          }],  # OS=="linux"
        ],
      }],
    ],
    'msvs_cygwin_shell': 0, # Strangely setting it to 1 would make building under cygwin fail.
    'msvs_disabled_warnings': [
      4189,  # local variable is initialized but not referenced
      4819,  # The file contains a character that cannot be represented in the current code page
      4996,  # (atlapp.h) 'GetVersionExW': was declared deprecated
    ],
    'msvs_settings': {
      'VCCLCompilerTool': {
        # Programs that use the Standard C++ library must be compiled with C++
        # exception handling enabled.
        # http://support.microsoft.com/kb/154419
        'ExceptionHandling': 1,
      },
      'VCLinkerTool': {
        'AdditionalOptions': [
          # ATL 8.0 included in WDK 7.1 makes the linker to generate following
          # warnings:
          #   - warning LNK4254: section 'ATL' (50000040) merged into
          #     '.rdata' (40000040) with different attributes
          #   - warning LNK4078: multiple 'ATL' sections found with
          #     different attributes
          '/ignore:4254',
          '/ignore:4078',
          # views_chromiumcontent.lib generates this warning because it's
          # symobls are defined as dllexport but used as static library:
          #   - warning LNK4217: locally defined symbol imported in function
          #   - warning LNK4049: locally defined symbol imported
          '/ignore:4217',
          '/ignore:4049',
        ],
      },
    },
    'xcode_settings': {
      'DEBUG_INFORMATION_FORMAT': 'dwarf-with-dsym',
    },
  },
  'conditions': [
    # Settings to compile with clang under OS X.
    ['clang==1', {
      'target_defaults': {
        'cflags_cc': [
          # Use gnu++11 instead of c++11 here, see:
          # https://code.google.com/p/chromium/issues/detail?id=224515
          '-std=gnu++11',
        ],
        'xcode_settings': {
          'CC': '/usr/bin/clang',
          'LDPLUSPLUS': '/usr/bin/clang++',
          'OTHER_CPLUSPLUSFLAGS': [
            '$(inherited)', '-std=gnu++11'
          ],
          'OTHER_CFLAGS': [
            '-fcolor-diagnostics',
          ],

          'GCC_C_LANGUAGE_STANDARD': 'c99',  # -std=c99
        },
      },
    }],  # clang==1
    # The breakdpad on Windows assumes Debug_x64 and Release_x64 configurations.
    ['OS=="win"', {
      'target_defaults': {
        'configurations': {
          'Debug_x64': {
          },
          'Release_x64': {
          },
        },
      },
    }],  # OS=="win"
    # The breakdpad on Mac assumes Release_Base configuration.
    ['OS=="mac"', {
      'target_defaults': {
        'configurations': {
          'Release_Base': {
          },
        },
      },
    }],  # OS=="mac"
    # The breakpad on Linux needs the binary to be built with -g to generate
    # unmangled symbols.
    ['OS=="linux"', {
      'target_defaults': {
        'cflags': [ '-g' ],
      },
    }],
  ],
}
