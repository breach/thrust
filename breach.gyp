# Copyright (c) 2013 Stanislas Polu. All rights reserved.

{
  'variables': {
    'breach_version': '0.0.1',
  },
  'targets': [
    {
      'target_name': 'breach_lib',
      'type': 'shared_library',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'defines': ['BREACH_VERSION="<(breach_version)"'],
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        'breach_resources',
        '<(DEPTH)/content/content.gyp:content_app',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_common',
        '<(DEPTH)/content/content.gyp:content_gpu',
        '<(DEPTH)/content/content.gyp:content_plugin',
        '<(DEPTH)/content/content.gyp:content_ppapi_plugin',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/content/content.gyp:content_utility',
        '<(DEPTH)/content/content.gyp:content_worker',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/ipc/ipc.gyp:ipc',
        '<(DEPTH)/media/media.gyp:media',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/net/net.gyp:net_resources',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/url/url.gyp:url_lib',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_resources',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_support',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party/WebKit/Source',
        '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/public',
        '<(SHARED_INTERMEDIATE_DIR)/webkit',
        '<(SHARED_INTERMEDIATE_DIR)/webkit/bindings',
      ],
      'sources': [
        'app/breach_main_delegate.cc',
        'app/breach_main_delegate.h',
        'browser/breach_content_browser_client.cc',
        'browser/breach_content_browser_client.h',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
      },
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            # For FT_Init_FreeType and friends.
            '<(DEPTH)/build/linux/system.gyp:freetype2',
            '<(DEPTH)/build/linux/system.gyp:gtk',
            '<(DEPTH)/build/linux/system.gyp:gtkprint',
          ],
        }],
        ['OS=="win"', {
          'resource_include_dirs': [
            '<(SHARED_INTERMEDIATE_DIR)/webkit',
          ],
          'dependencies': [
            '<(DEPTH)/webkit/webkit_resources.gyp:webkit_strings',   
          ],
          'configurations': {
            'Debug_Base': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                },
              },
            },
          },
          'msvs_disabled_warnings': [ 4800 ],
        }],  # OS=="win"
      ],
    },
    {
      'target_name': 'breach_resources',
      'type': 'none',
      'dependencies': [
        'generate_breach_resources',
      ],
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/breach',
      },
      'includes': [ '../build/grit_target.gypi' ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/breach/breach_resources.pak'
          ],
        },
      ],
    },
    {
      'target_name': 'generate_breach_resources',
      'type': 'none',
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/breach',
      },
      'actions': [
        {
          'action_name': 'breach_resources',
          'variables': {
            'grit_grd_file': 'breach_resources.grd',
          },
          'includes': [ '../build/grit_action.gypi' ],
        },
      ],
    },
    {
      # We build a minimal set of resources so WebKit in breach has
      # access to necessary resources.
      'target_name': 'breach_pak',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        'breach_resources',
        '<(DEPTH)/net/net.gyp:net_resources',
        '<(DEPTH)/ui/base/strings/ui_strings.gyp:ui_strings',
        '<(DEPTH)/ui/ui.gyp:ui_resources',
        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_resources',
        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_strings',
      ],
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_breach_pack',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/breach/breach_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/app_locale_settings/app_locale_settings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/webui_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_strings/ui_strings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_chromium_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_strings_en-US.pak',
            ],
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/breach.pak',
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)',
                     '<@(pak_inputs)'],
        },
      ],
    },
    {
      'target_name': 'breach',
      'type': 'executable',
      'mac_bundle': 1,
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        'breach_lib',
        'breach_pak',
      ],
      'include_dirs': [
        '<(DEPTH)',
      ],
      'sources': [
        '<(DEPTH)/content/app/startup_helper_win.cc',
        'app/breach_main.cc',
      ],
      'mac_bundle_resources': [
        'app/app.icns',
        'app/app-Info.plist',
      ],
      # TODO(mark): Come up with a fancier way to do this.  It should only
      # be necessary to list app-Info.plist once, not the three times it is
      # listed here.
      'mac_bundle_resources!': [
        'app/app-Info.plist',
      ],
      'xcode_settings': {
        'INFOPLIST_FILE': 'app/app-Info.plist',
      },
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
        'VCManifestTool': {
          'AdditionalManifestFiles': 'app/shell.exe.manifest',
        },
      },
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS=="win"', {
          'sources': [
            'app/shell.rc',
          ],
          'configurations': {
            'Debug_Base': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                },
              },
            },
          },
        }],  # OS=="win"
        ['OS == "win" or toolkit_uses_gtk == 1', {
          'dependencies': [
            '<(DEPTH)/sandbox/sandbox.gyp:sandbox',
          ],
        }],  # OS=="win" or toolkit_uses_gtk == 1
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '<(DEPTH)/build/linux/system.gyp:gtk',
          ],
        }],  # toolkit_uses_gtk
        ['OS=="mac"', {
          'product_name': '<(breach_product_name)',
          'dependencies!': [
            'breach_lib',
          ],
          'dependencies': [
            'breach_framework',
            'breach_helper_app',
          ],
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/<(breach_product_name).app/Contents/Frameworks',
              'files': [
                '<(PRODUCT_DIR)/<(breach_product_name) Helper.app',
              ],
            },
          ],
          'postbuilds': [
            {
              'postbuild_name': 'Copy <(breach_product_name) Framework.framework',
              'action': [
                '<(DEPTH)/build/mac/copy_framework_unversioned.sh',
                '${BUILT_PRODUCTS_DIR}/<(breach_product_name) Framework.framework',
                '${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/Frameworks',
              ],
            },
            {
              'postbuild_name': 'Fix Framework Link',
              'action': [
                'install_name_tool',
                '-change',
                '/Library/Frameworks/<(breach_product_name) Framework.framework/Versions/A/<(breach_product_name) Framework',
                '@executable_path/../Frameworks/<(breach_product_name) Framework.framework/<(breach_product_name) Framework',
                '${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}'
              ],
            },
            {
              # Modify the Info.plist as needed.
              'postbuild_name': 'Tweak Info.plist',
              'action': ['<(DEPTH)/build/mac/tweak_info_plist.py',
                         '--scm=1',
                         '--version=<(breach_version)'],
            },
            {
              # This postbuid step is responsible for creating the following
              # helpers:
              #
              # Content Shell Helper EH.app and Content Shell Helper NP.app are
              # created from Content Shell Helper.app.
              #
              # The EH helper is marked for an executable heap. The NP helper
              # is marked for no PIE (ASLR).
              'postbuild_name': 'Make More Helpers',
              'action': [
                '<(DEPTH)/build/mac/make_more_helpers.sh',
                'Frameworks',
                '<(breach_product_name)',
              ],
            },
            {
              # Make sure there isn't any Objective-C in the shell's
              # executable.
              'postbuild_name': 'Verify No Objective-C',
              'action': [
                '<(DEPTH)/build/mac/verify_no_objc.sh',
              ],
            },
          ],
        }],  # OS=="mac"
      ],
    },
  ],
  'conditions': [
    ['OS=="mac"', {
      'targets': [
        {
          'target_name': 'breach_framework',
          'type': 'shared_library',
          'product_name': '<(breach_product_name) Framework',
          'mac_bundle': 1,
          'mac_bundle_resources': [
            'app/English.lproj/HttpAuth.xib',
            'app/English.lproj/MainMenu.xib',
            '<(PRODUCT_DIR)/breach.pak'
          ],
          'dependencies': [
            'breach_lib',
          ],
          'include_dirs': [
            '<(DEPTH)',
          ],
          'sources': [
            'app/breach_main_mac.cc',
            'app/breach_main_mac.h',
          ],
          'copies': [
            {
              # Copy FFmpeg binaries for audio/video support.
              'destination': '<(PRODUCT_DIR)/$(CONTENTS_FOLDER_PATH)/Libraries',
              'files': [
                '<(PRODUCT_DIR)/ffmpegsumo.so',
              ],
            },
          ],
        },  # target breach_framework
        {
          'target_name': 'breach_helper_app',
          'type': 'executable',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'product_name': '<(breach_product_name) Helper',
          'mac_bundle': 1,
          'dependencies': [
            'breach_framework',
          ],
          'sources': [
            'app/breach_main.cc',
            'app/helper-Info.plist',
          ],
          # TODO(mark): Come up with a fancier way to do this.  It should only
          # be necessary to list helper-Info.plist once, not the three times it
          # is listed here.
          'mac_bundle_resources!': [
            'app/helper-Info.plist',
          ],
          # TODO(mark): For now, don't put any resources into this app.  Its
          # resources directory will be a symbolic link to the browser app's
          # resources directory.
          'mac_bundle_resources/': [
            ['exclude', '.*'],
          ],
          'xcode_settings': {
            'INFOPLIST_FILE': 'app/helper-Info.plist',
          },
          'postbuilds': [
            {
              # The framework defines its load-time path
              # (DYLIB_INSTALL_NAME_BASE) relative to the main executable
              # (chrome).  A different relative path needs to be used in
              # breach_helper_app.
              'postbuild_name': 'Fix Framework Link',
              'action': [
                'install_name_tool',
                '-change',
                '/Library/Frameworks/<(breach_product_name) Framework.framework/Versions/A/<(breach_product_name) Framework',
                '@executable_path/../../../../Frameworks/<(breach_product_name) Framework.framework/<(breach_product_name) Framework',
                '${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}'
              ],
            },
            {
              # Modify the Info.plist as needed.  The script explains why this
              # is needed.  This is also done in the chrome and chrome_dll
              # targets.  In this case, --breakpad=0, --keystone=0, and --scm=0
              # are used because Breakpad, Keystone, and SCM keys are
              # never placed into the helper.
              'postbuild_name': 'Tweak Info.plist',
              'action': ['<(DEPTH)/build/mac/tweak_info_plist.py',
                         '--breakpad=0',
                         '--keystone=0',
                         '--scm=0',
                         '--version=<(breach_version)'],
            },
            {
              # Make sure there isn't any Objective-C in the helper app's
              # executable.
              'postbuild_name': 'Verify No Objective-C',
              'action': [
                '<(DEPTH)/build/mac/verify_no_objc.sh',
              ],
            },
          ],
          'conditions': [
            ['component=="shared_library"', {
              'xcode_settings': {
                'LD_RUNPATH_SEARCH_PATHS': [
                  # Get back from Breach.app/Contents/Frameworks/
                  #                                 Helper.app/Contents/MacOS
                  '@loader_path/../../../../../..',
                ],
              },
            }],
          ],
        },  # target breach_helper_app
      ],
    }],  # OS=="mac"
  ]
}

